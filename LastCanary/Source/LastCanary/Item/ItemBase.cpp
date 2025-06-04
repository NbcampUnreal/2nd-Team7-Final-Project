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

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	RootComponent = RootSceneComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	MeshComponent->SetupAttachment(RootSceneComponent);

	SkeletalMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComponent"));
	SkeletalMeshComponent->SetupAttachment(RootSceneComponent);

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
			ApplyItemDataFromTable();
		}
	}

	//if (HasAuthority() && bUsingSkeletalMesh && SkeletalMeshComponent)
	//{
	//	// 물리 시뮬레이션 시 액터 위치 동기화
	//	SkeletalMeshComponent->SetNotifyRigidBodyCollision(true);

	//	// 주기적으로 위치 동기화 (타이머 사용)
	//	GetWorld()->GetTimerManager().SetTimer(PhysicsSyncTimer,
	//		this, &AItemBase::SyncPhysicsToActor, 0.05f, true);
	//}
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

void AItemBase::OnOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor);
	if (Player && Player->OwnedTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Character.Player"))))
	{
		// TODO : UI 매니저를 통한 UI 출력 또는 아웃라이너 변경
		// ShowPickUpPrompt(ture);
	}
}

void AItemBase::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	ABaseCharacter* Player = Cast<ABaseCharacter>(OtherActor);
	if (Player && Player->OwnedTags.HasTagExact(FGameplayTag::RequestGameplayTag(FName("Character.Player"))))
	{
		// TODO : UI 매니저를 통한 UI 출력 또는 아웃라이너 변경
		// ShowPickUpPrompt(false);
	}
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

	if (ItemData.bIsResourceItem)
	{
		LOG_Frame_WARNING(TEXT("이 아이템은 자원입니다. 카테고리: %d, 점수: %d"),
			static_cast<int32>(ItemData.Category), ItemData.BaseScore);
	}

	ApplyCollisionSettings();

	OnItemStateChanged.Broadcast();
}

void AItemBase::SetupMeshComponents()
{
	if (ItemData.SkeletalMesh)
	{
		bUsingSkeletalMesh = true;
		SkeletalMeshComponent->SetSkeletalMesh(ItemData.SkeletalMesh);

		SetMeshComponentActive(SkeletalMeshComponent, MeshComponent);
	}
	else if (ItemData.StaticMesh)
	{
		bUsingSkeletalMesh = false;
		MeshComponent->SetStaticMesh(ItemData.StaticMesh);

		SetMeshComponentActive(MeshComponent, SkeletalMeshComponent);
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
	else if (MeshComponent)
	{
		return MeshComponent;
	}
	return nullptr;
}

UStaticMeshComponent* AItemBase::GetMeshComponent() const
{
	if (bUsingSkeletalMesh)
	{
		return nullptr;
	}
	return MeshComponent;
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

				if (ItemData.StaticMesh && MeshComponent)
				{
					MeshComponent->SetStaticMesh(ItemData.StaticMesh);
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
			return FString::Printf(TEXT("%s 습득 (x%d)"),
				*Found->ItemName.ToString(), Quantity);
		}
	}

	return FString::Printf(TEXT("%s 습득 (x%d)"),
		*ItemRowName.ToString(), Quantity);
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

	LOG_Item_WARNING(TEXT("[ApplyCollisionSettings] === 시작 === 아이템: %s, 메시 타입: %s, bIgnoreCharacterCollision: %s"),
		*GetName(),
		bUsingSkeletalMesh ? TEXT("SkeletalMesh") : TEXT("StaticMesh"),
		bIgnoreCharacterCollision ? TEXT("true") : TEXT("false"));

	// 통합된 충돌 설정
	ActiveMeshComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	ActiveMeshComp->SetCollisionObjectType(ECC_WorldDynamic);
	ActiveMeshComp->SetCollisionResponseToAllChannels(ECR_Block);

	if (bIgnoreCharacterCollision)
	{
		ActiveMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
		LOG_Item_WARNING(TEXT("[ApplyCollisionSettings] 캐릭터 충돌 무시 설정 적용: %s"), *GetName());
	}
	else
	{
		ActiveMeshComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
		LOG_Item_WARNING(TEXT("[ApplyCollisionSettings] 캐릭터 충돌 활성화 설정 적용: %s"), *GetName());
	}

	// 기타 채널 설정
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Block);
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	ActiveMeshComp->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	// 설정 후 검증
	ECollisionResponse PawnResponse = ActiveMeshComp->GetCollisionResponseToChannel(ECC_Pawn);
	LOG_Item_WARNING(TEXT("[ApplyCollisionSettings] ✅ 설정 완료 - 아이템: %s, 메시 타입: %s, Pawn 채널 응답: %d"),
		*GetName(),
		bUsingSkeletalMesh ? TEXT("SkeletalMesh") : TEXT("StaticMesh"),
		(int32)PawnResponse);
}

//void AItemBase::Multicast_SetupVisualEffects_Implementation(FVector ThrowDirection, float ThrowVelocity, FVector ThrowImpulse)
//{
//	// 서버에서는 실행하지 않음 (이미 물리 적용됨)
//	if (HasAuthority())
//	{
//		return;
//	}
//
//	if (UPrimitiveComponent* ActiveMeshComp = GetActiveMeshComponent())
//	{
//		// 클라이언트에서는 물리 시뮬레이션 비활성화
//		ActiveMeshComp->SetSimulatePhysics(false);
//
//		// 스켈레탈 메시인 경우 애니메이션 인스턴스만 제거
//		if (USkeletalMeshComponent* SkeletalMeshComp = Cast<USkeletalMeshComponent>(ActiveMeshComp))
//		{
//			if (UAnimInstance* AnimInstance = SkeletalMeshComp->GetAnimInstance())
//			{
//				SkeletalMeshComp->SetAnimInstanceClass(nullptr);
//			}
//		}
//
//		ActiveMeshComp->SetSimulatePhysics(true);
//		ActiveMeshComp->SetLinearDamping(0.1f);
//		ActiveMeshComp->SetAngularDamping(0.1f);
//	}
//	else
//	{
//		LOG_Item_WARNING(TEXT("[AItemBase::Multicast_SetupVisualEffects] 클라이언트에서 활성화된 메시 컴포넌트를 찾을 수 없음"));
//	}
//}
