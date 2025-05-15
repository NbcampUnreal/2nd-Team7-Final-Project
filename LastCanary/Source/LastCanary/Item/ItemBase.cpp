// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/ItemBase.h"
#include "Character/BaseCharacter.h"
#include "Components/SphereComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
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

	bIsEquipped = false;

	// UI 관리는 UI 매니저에서 할것으로 예상
	//PickupWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupWidgetComponent"));
	//PickupWidgetComponent->SetupAttachment(MeshComponent);

	//PickupWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	//PickupWidgetComponent->SetDrawSize(FVector2D(200, 50));
	//PickupWidgetComponent->SetVisibility(false);

	Quantity = 1;
	Durability = 100.f;
}

void AItemBase::BeginPlay()
{
	Super::BeginPlay();
	
	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] World is null!"));
		return;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] GameInstance is null!"));
		return;
	}

	ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!GISubsystem)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] LCGameInstance is null"));
		return;
	}

	ItemDataTable = GISubsystem->ItemDataTable;
	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] ItemDataTable is null"));
		return;
	}

	ApplyItemDataFromTable();
}

#if WITH_EDITOR
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
	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::ApplyItemDataFromTable] ItemDataTable is null!"));
		return;
	}

	FItemDataRow* Found = ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("ApplyItemDataFromTable"));
	if (Found)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::ApplyItemDataFromTable] ItemData is null!"));
	}

	ItemData = *Found;

	if (MeshComponent && ItemData.StaticMesh)
	{
		MeshComponent->SetStaticMesh(ItemData.StaticMesh);
	}
}

void AItemBase::UseItem()
{
	// TODO : 조건에 따라 아이템 타입에 따른 사용함수를 구현하거나 혹은 상속받은 곳에서 구현이 필요할 것으로 예상
	OnItemStateChanged.Broadcast();
}

bool AItemBase::IsCollectible() const
{
	// TODO : 태그를 통해 판정하는 로직 작성 필요
	return false;
}

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
