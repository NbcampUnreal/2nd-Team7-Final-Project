#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Item/ResourceItem/ResourceItemSpawnPoint.h"
#include "Item/ItemBase.h"
#include "Item/ItemSpawnerComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Manager/LCTimeManager.h"
#include "LastCanary.h"

AResourceItemSpawnManager::AResourceItemSpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bSpawnOnBeginPlay = true;

	ItemSpawnerComponent = CreateDefaultSubobject<UItemSpawnerComponent>(TEXT("ItemSpawnerComponent"));
}

void AResourceItemSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() == true)
	{
		// 현재 맵에 따른 지역 태그 설정
		SetCurrentMapRegionTag();

		if (ALCTimeManager* TimeManager = Cast<ALCTimeManager>(UGameplayStatics::GetActorOfClass(GetWorld(), ALCTimeManager::StaticClass())))
		{
			TimeManager->OnTimePhaseChanged.AddDynamic(this, &AResourceItemSpawnManager::OnDayNightChanged);
		}

		if (bSpawnOnBeginPlay)
		{
			SpawnItemsForTheme();
		}
	}
}

void AResourceItemSpawnManager::OnDayNightChanged(EDayPhase NewPhase)
{
	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceItemSpawnPoint::StaticClass(), FoundPoints);

	// 태그 기반으로 스폰 가능한 아이템 목록 가져오기
	TArray<FName> AvailableItems = GetSpawnableItemsByTags();

	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			const auto Condition = Point->SpawnTimeCondition;
			if (Condition == ESpawnTimeCondition::Always)
			{
				if (!Point->bHasSpawnedAlwaysItem)
				{
					Point->TrySpawnItemFromManager(AvailableItems, ItemSpawnerComponent);
				}
				else
				{
					LOG_Item_WARNING(TEXT("  Always point - already spawned"));
				}
				continue;
			}

			// 해당 시간에만 스폰하는 포인트 처리
			if ((NewPhase == EDayPhase::Night && Condition == ESpawnTimeCondition::NightOnly) ||
				(NewPhase == EDayPhase::Day && Condition == ESpawnTimeCondition::DayOnly))
			{
				Point->TrySpawnItemFromManager(AvailableItems, ItemSpawnerComponent);
			}

			// 반대 시간대면 디스폰
			if ((NewPhase == EDayPhase::Night && Condition == ESpawnTimeCondition::DayOnly) ||
				(NewPhase == EDayPhase::Day && Condition == ESpawnTimeCondition::NightOnly))
			{
				Point->ClearSpawnedItem();
			}
		}
	}
}

void AResourceItemSpawnManager::SpawnItemByRow(FName ItemRowName)
{
	if (ItemSpawnerComponent)
	{
		AItemBase* SpawnedItem = ItemSpawnerComponent->CreateItem(ItemRowName, GetActorLocation());
	}
}

void AResourceItemSpawnManager::SpawnItemsForTheme()
{
	// 태그 기반으로 스폰 가능한 아이템 목록 가져오기
	TArray<FName> SpawnableItems = GetSpawnableItemsByTags();

	if (SpawnableItems.Num() == 0)
	{
		LOG_Item_WARNING(TEXT("No spawnable items found for current region tags"));
		return;
	}

	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceItemSpawnPoint::StaticClass(), FoundPoints);

	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			Point->TrySpawnItemFromManager(SpawnableItems, ItemSpawnerComponent);
		}
	}
}

TArray<FName> AResourceItemSpawnManager::GetSpawnableItemsByTags() const
{
	TArray<FName> SpawnableItems;

	UWorld* World = GetWorld();
	if (!World)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] World is null!"));
		return SpawnableItems;
	}

	UGameInstance* GI = World->GetGameInstance();
	if (!GI)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] GameInstance is null!"));
		return SpawnableItems;
	}

	ULCGameInstanceSubsystem* GISubsystem = GI->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!GISubsystem)
	{
		LOG_Item_WARNING(TEXT("[InventoryComponentBase::BeginPlay] LCGameInstance is null"));
		return SpawnableItems;
	}
	TArray<FName> AllItemNames = GISubsystem->ItemDataTable->GetRowNames();

	for (const FName& ItemName : AllItemNames)
	{
		if (FItemDataRow* ItemData = GISubsystem->GetItemDataByRowName(ItemName))
		{
			if (ItemData->AllowedSpawnMaps.IsEmpty())
			{
				continue;
			}

			bool bRegionMatch = ItemData->AllowedSpawnMaps.IsEmpty() || ItemData->AllowedSpawnMaps.HasAnyExact(CurrentRegionTags);

			if (bRegionMatch)
			{
				SpawnableItems.Add(ItemName);
				LOG_Item_WARNING(TEXT("[ResourceItemSpawnManager] Added spawnable item: %s"), *ItemName.ToString());
			}
		}
	}

	return SpawnableItems;
}

void AResourceItemSpawnManager::SetCurrentRegionTag(const FString& MapName)
{
	CurrentRegionTags.Reset();

	FString TagName = FString::Printf(TEXT("ItemSpawn.Map.%s"), *MapName);
	FGameplayTag RegionTag = FGameplayTag::RequestGameplayTag(*TagName);

	if (RegionTag.IsValid())
	{
		CurrentRegionTags.AddTag(RegionTag);
		UE_LOG(LogTemp, Log, TEXT("[ResourceItemSpawnManager] Set region tag: %s"), *TagName);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ResourceItemSpawnManager] Invalid region tag: %s"), *TagName);
	}
}

void AResourceItemSpawnManager::SetCurrentMapRegionTag()
{
	// 현재 맵 이름 가져오기
	FString CurrentMapName = GetWorld()->GetMapName();

	// 맵 이름을 기반으로 태그 설정
	if (CurrentMapName.Contains("Cave") || CurrentMapName.Contains("cave"))
	{
		CurrentRegionTags.AddTag(FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Cave"));
	}
	else if (CurrentMapName.Contains("Forest") || CurrentMapName.Contains("forest"))
	{
		CurrentRegionTags.AddTag(FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Forest"));
	}
	else if (CurrentMapName.Contains("Ruins") || CurrentMapName.Contains("ruins"))
	{
		CurrentRegionTags.AddTag(FGameplayTag::RequestGameplayTag("ItemSpawn.Map.Ruins"));
	}
}
