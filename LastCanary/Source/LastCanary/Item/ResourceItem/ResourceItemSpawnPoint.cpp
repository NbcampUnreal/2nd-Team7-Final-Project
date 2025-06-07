#include "Item/ResourceItem/ResourceItemSpawnPoint.h"
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Manager/LCTimeManager.h" // 낮밤 체크용
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LastCanary.h"

AResourceItemSpawnPoint::AResourceItemSpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;

	SpawnedResourceItem = nullptr;

	bHasSpawnedAlwaysItem = false;
}

void AResourceItemSpawnPoint::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 스폰
	//if (HasAuthority())
	//{
	//	TrySpawnItem();
	//}
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

	AItemBase* SpawnedItem = GetWorld()->SpawnActorDeferred<AItemBase>(
		ItemClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (SpawnedItem)
	{
		SpawnedItem->ItemRowName = ItemRowName;
		SpawnedItem->FinishSpawning(SpawnTransform);
		SpawnedItem->ApplyItemDataFromTable();

		SpawnedResourceItem = SpawnedItem;
	}
}

void AResourceItemSpawnPoint::TrySpawnItemFromManager(const TArray<FName>& AvailableItems, UItemSpawnerComponent* SpawnerComponent)
{
	// 이미 스폰된 아이템이 있으면 스킵
	if (SpawnedResourceItem && IsValid(SpawnedResourceItem))
	{
		return;
	}

	if (!SpawnerComponent)
	{
		LOG_Item_WARNING(TEXT("SpawnerComponent is null in %s"), *GetName());
		return;
	}

	// 확률 체크
	if (FMath::FRand() > SpawnProbability)
	{
		return;
	}

	// Always 조건이고 이미 스폰했으면 스킵
	if (SpawnTimeCondition == ESpawnTimeCondition::Always && bHasSpawnedAlwaysItem)
	{
		return;
	}

	// 시간 조건 체크
	if (!CheckTimeCondition())
	{
		return;
	}

	// 시간 조건에 맞는 아이템들만 필터링
	TArray<FName> FilteredItems = FilterItemsByTimeCondition(AvailableItems);

	if (FilteredItems.Num() == 0)
	{
		return;
	}

	// 랜덤 선택
	int32 Index = UKismetMathLibrary::RandomIntegerInRange(0, FilteredItems.Num() - 1);
	FName SelectedRow = FilteredItems[Index];

	// ItemSpawnerComponent를 사용하여 아이템 생성
	AItemBase* SpawnedItem = SpawnerComponent->CreateItemAtLocation(SelectedRow, GetActorLocation());

	if (SpawnedItem)
	{
		SpawnedResourceItem = SpawnedItem;

		// Always 조건이면 플래그 설정
		if (SpawnTimeCondition == ESpawnTimeCondition::Always)
		{
			bHasSpawnedAlwaysItem = true;
		}
	}
}

void AResourceItemSpawnPoint::ClearSpawnedItem()
{
	if (SpawnedResourceItem && IsValid(SpawnedResourceItem))
	{
		SpawnedResourceItem->Destroy();
		SpawnedResourceItem = nullptr;
	}
}

bool AResourceItemSpawnPoint::CheckTimeCondition() const
{
	if (SpawnTimeCondition == ESpawnTimeCondition::Always)
	{
		return true;
	}

	if (const ALCTimeManager* TimeManager = Cast<ALCTimeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ALCTimeManager::StaticClass())))
	{
		switch (SpawnTimeCondition)
		{
		case ESpawnTimeCondition::DayOnly:
			return TimeManager->CurrentPhase == EDayPhase::Day;
		case ESpawnTimeCondition::NightOnly:
			return TimeManager->CurrentPhase == EDayPhase::Night;
		default:
			return true;
		}
	}

	return true;
}

TArray<FName> AResourceItemSpawnPoint::FilterItemsByTimeCondition(const TArray<FName>& AvailableItems) const
{
	TArray<FName> FilteredItems;

	UWorld* World = GetWorld();
	if (!World) return FilteredItems;

	UGameInstance* GI = World->GetGameInstance();
	if (!GI) return FilteredItems;

	ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!GISubsystem) return FilteredItems;

	FGameplayTag AlwaysTag = FGameplayTag::RequestGameplayTag("ItemSpawn.Time.Always", false);
	FGameplayTag DayTag = FGameplayTag::RequestGameplayTag("ItemSpawn.Time.Day", false);
	FGameplayTag NightTag = FGameplayTag::RequestGameplayTag("ItemSpawn.Time.Night", false);

	for (const FName& ItemName : AvailableItems)
	{
		if (FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(ItemName))
		{
			bool bTimeMatch = false;

			// 시간 태그가 비어있으면 어떤 포인트에서도 스폰 안 함
			if (ItemData->TimeConditions.IsEmpty())
			{
				bTimeMatch = false;
				LOG_Item_WARNING(TEXT("  Time Match: FALSE (Empty time tags - REJECTED)"));
			}
			else
			{
				// 각 포인트 타입에 정확히 맞는 태그만
				switch (SpawnTimeCondition)
				{
				case ESpawnTimeCondition::Always:
					// Always 포인트는 Always 태그만
					bTimeMatch = AlwaysTag.IsValid() && ItemData->TimeConditions.HasTagExact(AlwaysTag);
					break;

				case ESpawnTimeCondition::DayOnly:
					// Day 포인트는 Day 태그만 (Always 태그 제외)
					bTimeMatch = DayTag.IsValid() && ItemData->TimeConditions.HasTagExact(DayTag);
					break;

				case ESpawnTimeCondition::NightOnly:
					// Night 포인트는 Night 태그만 (Always 태그 제외)
					bTimeMatch = NightTag.IsValid() && ItemData->TimeConditions.HasTagExact(NightTag);
					break;

				default:
					bTimeMatch = false;
					LOG_Item_WARNING(TEXT("  Unknown condition - Match: FALSE"));
					break;
				}
			}

			if (bTimeMatch)
			{
				FilteredItems.Add(ItemName);
			}
		}
	}
	return FilteredItems;
}
