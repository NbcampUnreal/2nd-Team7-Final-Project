#include "Item/ResourceItem/ResourceItemSpawnPoint.h"
#include "Item/ResourceItem/ResourceItemBase.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Manager/LCTimeManager.h" // 낮밤 체크용

AResourceItemSpawnPoint::AResourceItemSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SpawnedResourceItem = nullptr;
}

void AResourceItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 스폰
	if (HasAuthority())
	{
		TrySpawnItem();
	}
}

void AResourceItemSpawnPoint::TrySpawnItem()
{
	if (SpawnedResourceItem && IsValid(SpawnedResourceItem))
	{
		return;
	}

	if (FMath::FRand() > SpawnProbability)
	{
		return;
	}

	// 시간 조건 확인
	if (SpawnTimeCondition != ESpawnTimeCondition::Always)
	{
		if (const ALCTimeManager* TimeManager = Cast<ALCTimeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ALCTimeManager::StaticClass())))
		{
			if (SpawnTimeCondition == ESpawnTimeCondition::NightOnly && TimeManager->CurrentPhase != EDayPhase::Night)
			{
				return;
			}
			if (SpawnTimeCondition == ESpawnTimeCondition::DayOnly && TimeManager->CurrentPhase != EDayPhase::Day)
			{
				return;
			}
		}
	}

	// 랜덤한 아이템 RowName 선택
	if (PossibleItems.Num() == 0)
	{
		return;
	}

	int32 Index = UKismetMathLibrary::RandomIntegerInRange(0, PossibleItems.Num() - 1);
	FName SelectedRow = PossibleItems[Index];

	SpawnItemByRow(SelectedRow);
}

void AResourceItemSpawnPoint::SpawnItemByRow(FName ItemRowName)
{
	if (ItemClass == nullptr)
	{
		return;
	}

	FTransform SpawnTransform = GetActorTransform();

	AResourceItemBase* SpawnedItem = GetWorld()->SpawnActorDeferred<AResourceItemBase>(
		ItemClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (SpawnedItem)
	{
		SpawnedItem->ItemRowName = ItemRowName;
		SpawnedItem->FinishSpawning(SpawnTransform);
		SpawnedItem->ApplyItemDataFromTable();

		SpawnedResourceItem = SpawnedItem;
	}
}