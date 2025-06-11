#include "Item/ItemBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "Character/BaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	RootComponent = StaticMeshComponent;

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(StaticMeshComponent);

	SkeletalMeshComponent->SetVisibility(false);
	SkeletalMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bReplicates = true;
	bNetUseOwnerRelevancy = false;

	SetReplicatingMovement(true);

	bIsEquipped = false;
	bUsingSkeletalMesh = false;
	Quantity = 1;
	Durability = 100.f;
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		UWorld* World = GetWorld();
		if (!World)
		{
			LOG_Item_WARNING(TEXT("[AItemBase::BeginPlay] World is null!"));
			return;
		}

		UGameInstance* GI = World->GetGameInstance();
		if (!GI)
		{
			LOG_Item_WARNING(TEXT("[AItemBase::BeginPlay] GameInstance is null!"));
			return;
		}

		ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
		if (!GISubsystem)
		{
			LOG_Item_WARNING(TEXT("[AItemBase::BeginPlay] LCGameInstanceSubsystem is null"));
			return;
		}

		ItemDataTable = GISubsystem->ItemDataTable;
		if (!ItemDataTable)
		{
			LOG_Item_WARNING(TEXT("[AItemBase::BeginPlay] ItemDataTable is null"));
			return;
		}

		if (!ItemRowName.IsNone() && GetOwner() != GetAttachParentActor())
		{
			//ApplyItemDataFromTable();
		}
	}

	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(PhysicsLocationSyncTimer,
			this, &AItemBase::SyncPhysicsLocationToActor, 0.1f, true);
	}

	EnableStencilForAllMeshes(3);
}

void AItemBase::OnRepDurability()
{
	if (FMath::IsNearlyZero(Durability) || Durability <= 0.0f)
	{
		Durability = 0.0f;
	}
	else if (Durability > 100.0f)
	{
		Durability = 100.0f;
	}

	OnItemStateChanged.Broadcast();
}

void AItemBase::ApplyItemDataFromTable()
{
	if (ItemRowName.IsNone())
	{
		LOG_Item_WARNING(TEXT("[AItemBase::ApplyItemDataFromTable] ItemRowName이 설정되지 않았습니다!"));
		return;
	}

	if (!ItemDataTable)
	{
		if (UGameInstance* GI = GetGameInstance())
		{
			if (ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				ItemDataTable = GISubsystem->ItemDataTable;
			}
		}
	}

	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[AItemBase::ApplyItemDataFromTable] ItemDataTable is null!"));
		return;
	}

	FItemDataRow* Found = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("ApplyItemDataFromTable"));
	if (!Found)
	{
		LOG_Item_WARNING(TEXT("[AItemBase::ApplyItemDataFromTable] ItemData not found for: %s"),
			*ItemRowName.ToString());
		return;
	}

	ItemData = *Found;
	bIgnoreCharacterCollision = ItemData.bIgnoreCharacterCollision;

	SetupMeshComponents();

	/*if (ItemData.bIsResourceItem)
	{
		LOG_Frame_WARNING(TEXT("이 아이템은 자원입니다. 카테고리: %d, 점수: %d"),
			static_cast<int32>(ItemData.Category), ItemData.BaseScore);
	}*/

	ApplyCollisionSettings();

	OnItemStateChanged.Broadcast();
}

void AItemBase::SetupMeshComponents()
{
	if (ItemData.SkeletalMesh)
	{
		bUsingSkeletalMesh = true;
		SkeletalMeshComponent->SetSkeletalMesh(ItemData.SkeletalMesh);

		SetMeshComponentActive(SkeletalMeshComponent, StaticMeshComponent);
	}
	else if (ItemData.StaticMesh)
	{
		bUsingSkeletalMesh = false;
		StaticMeshComponent->SetStaticMesh(ItemData.StaticMesh);

		// ItemData 적용 후, 메시가 세팅된 다음 머티리얼 적용
		if (ItemData.OverrideMaterial)
		{
			if (StaticMeshComponent && StaticMeshComponent->GetStaticMesh())
			{
				StaticMeshComponent->SetMaterial(0, ItemData.OverrideMaterial);
			}
		}

		SetMeshComponentActive(StaticMeshComponent, SkeletalMeshComponent);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[SetupMeshComponents] 메시가 설정되지 않음: %s"), *ItemRowName.ToString());
	}
}

void AItemBase::SetMeshComponentActive(UPrimitiveComponent* ActiveComponent, UPrimitiveComponent* InactiveComponent)
{
	if (ActiveComponent)
	{
		ActiveComponent->SetVisibility(true);
		ActiveComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[AItemBase::SetMeshComponentActive] 활성화할 컴포넌트가 유효하지 않음"));
	}

	if (InactiveComponent)
	{
		InactiveComponent->SetVisibility(false);
		InactiveComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		LOG_Item_WARNING(TEXT("[SetMeshComponentActive] 비활성화할 컴포넌트가 유효하지 않음"));
	}
}

UPrimitiveComponent* AItemBase::GetActiveMeshComponent() const
{
	if (bUsingSkeletalMesh && SkeletalMeshComponent)
	{
		return SkeletalMeshComponent;
	}
	else if (StaticMeshComponent)
	{
		return StaticMeshComponent;
	}
	return nullptr;
}

UStaticMeshComponent* AItemBase::GetMeshComponent() const
{
	if (bUsingSkeletalMesh)
	{
		return nullptr;
	}
	return StaticMeshComponent;
}

USkeletalMeshComponent* AItemBase::GetSkeletalMeshComponent() const
{
	return SkeletalMeshComponent;
}

//void AItemBase::UseItem()
//{
//	// TODO : 조건에 따라 아이템 타입에 따른 사용함수를 구현하거나 혹은 상속받은 곳에서 구현이 필요할 것으로 예상
//	OnItemStateChanged.Broadcast();
//}

bool AItemBase::IsCollectible() const
{
	// TODO : 태그를 통해 판정하는 로직 작성 필요
	return false;
}

#if WITH_EDITOR
void AItemBase::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	static const FName RowNamePropName = GET_MEMBER_NAME_CHECKED(AItemBase, ItemRowName);

	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == RowNamePropName)
	{
		if (ItemDataTable)
		{
			FItemDataRow* Found = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("Editor Item Lookup"));
			if (Found)
			{
				ItemData = *Found;

				if (ItemData.StaticMesh && StaticMeshComponent)
				{
					StaticMeshComponent->SetStaticMesh(ItemData.StaticMesh);
				}
			}
			else
			{
				LOG_Item_WARNING(TEXT("[ItemBase::PostEditChangeProperty] 아이템 데이터 할당 실패 : %s"), *ItemData.ItemName.ToString());
			}
		}
	}
}
#endif

void AItemBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION_NOTIFY(AItemBase, ItemRowName, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AItemBase, Quantity);
	DOREPLIFETIME(AItemBase, bIsEquipped);
	DOREPLIFETIME_CONDITION_NOTIFY(AItemBase, Durability, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME(AItemBase, bIgnoreCharacterCollision);
}

void AItemBase::OnRepItemRowName()
{
	if (!ItemRowName.IsNone())
	{
		ApplyItemDataFromTable();

		if (bIsEquipped)
		{
			SetActorEnableCollision(false);
		}
	}
}

void AItemBase::Interact_Implementation(APlayerController* Interactor)
{
	if (!Interactor)
	{
		LOG_Item_WARNING(TEXT("[AItemBase::Interact_Implementation] Interactor가 nullptr입니다."));
		return;
	}

	if (ABaseCharacter* Character = Cast<ABaseCharacter>(Interactor->GetPawn()))
	{
		Character->TryPickupItem(this);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[AItemBase::Interact_Implementation] BaseCharacter를 찾을 수 없음"));
	}
}

FString AItemBase::GetInteractMessage_Implementation() const
{
	if (ItemRowName.IsNone())
	{
		return FString(TEXT("아이템 습득"));
	}

	if (ItemDataTable)
	{
		if (const FItemDataRow* Found = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("GetInteractMessage")))
		{
			return FString::Printf(TEXT("%s 습득 (x%d)"), *Found->ItemName.ToString(), Quantity);
		}
	}

	return FString::Printf(TEXT("%s 습득 (x%d)"), *ItemRowName.ToString(), Quantity);
}

void AItemBase::Server_TryPickupByPlayer_Implementation(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		LOG_Item_WARNING(TEXT("[AItemBase::Server_TryPickupByPlayer] PlayerController가 nullptr입니다."));
		return;
	}

	Internal_TryPickupByPlayer(PlayerController);
}

bool AItemBase::Internal_TryPickupByPlayer(APlayerController* PlayerController)
{
	if (!HasAuthority())
	{
		LOG_Item_WARNING(TEXT("[AItemBase::Internal_TryPickupByPlayer] Authority가 없습니다."));
		return false;
	}

	if (!PlayerController)
	{
		LOG_Item_WARNING(TEXT("[AItemBase::Internal_TryPickupByPlayer] PlayerController가 nullptr입니다."));
		return false;
	}

	ABaseCharacter* Character = Cast<ABaseCharacter>(PlayerController->GetPawn());
	if (!Character)
	{
		LOG_Item_WARNING(TEXT("[AItemBase::Internal_TryPickupByPlayer] BaseCharacter를 찾을 수 없습니다."));
		return false;
	}

	if (UBackpackInventoryComponent* BackpackInventory = Character->GetBackpackInventoryComponent())
	{
		if (BackpackInventory->TryAddItem(this))
		{
			return true;
		}
	}

	if (UToolbarInventoryComponent* ToolbarInventory = Character->GetToolbarInventoryComponent())
	{
		if (ToolbarInventory->TryAddItem(this))
		{
			return true;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("[AItemBase::Internal_TryPickupByPlayer] 인벤토리가 가득참: %s"),
		*ItemRowName.ToString());
	return false;
}

void AItemBase::ApplyCollisionSettings()
{
	UPrimitiveComponent* ActiveMeshComp = GetActiveMeshComponent();
	if (!ActiveMeshComp)
	{
		LOG_Item_WARNING(TEXT("[ApplyCollisionSettings] 활성화된 메시 컴포넌트가 없음: %s"), *GetName());
		return;
	}

	ActiveMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ActiveMeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	ActiveMeshComp->SetCollisionResponseToAllChannels(ECR_Block);

	if (bIgnoreCharacterCollision)
	{
		ActiveMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
	}
	else
	{
		ActiveMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	// 기타 채널 설정
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AItemBase::SyncPhysicsLocationToActor()
{
	if (!HasAuthority())
		return;

	if (UPrimitiveComponent* ActiveMeshComp = GetActiveMeshComponent())
	{
		if (ActiveMeshComp->IsSimulatingPhysics())
		{
			// 물리 컴포넌트의 위치를 액터 위치로 동기화
			FVector PhysicsLocation = ActiveMeshComp->GetComponentLocation();
			FVector ActorLocation = GetActorLocation();

			float Distance = FVector::Dist(PhysicsLocation, ActorLocation);
			if (Distance > 5.0f) 
			{
				SetActorLocation(PhysicsLocation);
				ForceNetUpdate();
			}
		}
		else
		{
			if (PhysicsLocationSyncTimer.IsValid())
			{
				GetWorld()->GetTimerManager().ClearTimer(PhysicsLocationSyncTimer);
			}
		}
	}
}

// EndPlay에서 타이머 정리
void AItemBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (PhysicsLocationSyncTimer.IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(PhysicsLocationSyncTimer);
	}

	Super::EndPlay(EndPlayReason);
}

void AItemBase::EnableStencilForAllMeshes(int32 StencilValue)
{
	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(StencilValue);
	}
}
