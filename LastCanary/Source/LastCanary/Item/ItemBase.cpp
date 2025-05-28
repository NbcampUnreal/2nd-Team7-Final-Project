// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemBase.h"
#include "Character/BaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

AItemBase::AItemBase()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("Collision"));
	SphereComponent->InitSphereRadius(50.0f);
	SphereComponent->SetCollisionProfileName(TEXT("Trigger"));
	SphereComponent->SetupAttachment(MeshComponent);

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, & AItemBase::OnOverlapBegin);
	SphereComponent->OnComponentEndOverlap.AddDynamic(this, &AItemBase::OnOverlapEnd);

	SphereComponent->SetCollisionObjectType(ECC_GameTraceChannel1);
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

	bReplicates = true;
	bNetUseOwnerRelevancy = true;

	bIsEquipped = false;

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

	// 🔥 클라이언트에서도 데이터 테이블 참조 확보
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

	if (MeshComponent && ItemData.StaticMesh)
	{
		MeshComponent->SetStaticMesh(ItemData.StaticMesh);
	}

	// 상태 변경 브로드캐스트
	OnItemStateChanged.Broadcast();
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
				LOG_Item_WARNING(TEXT("[ItemBase::PostEditChangeProperty] 아이템 데이터 할당 성공 : %s"), *ItemData.ItemName.ToString());

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

		LOG_Item_WARNING(TEXT("[AItemBase::OnRepItemRowName] 클라이언트에서 아이템 데이터 적용: %s"),
			*ItemRowName.ToString());
	}
}

UStaticMeshComponent* AItemBase::GetMeshComponent() const
{
	if (MeshComponent)
	{
		return MeshComponent;
	}
	return nullptr;
}
