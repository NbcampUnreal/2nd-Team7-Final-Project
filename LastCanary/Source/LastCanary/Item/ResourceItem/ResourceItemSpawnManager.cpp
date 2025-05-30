#include "Item/ResourceItem/ResourceItemSpawnManager.h"
#include "Item/ResourceItem/ResourceItemSpawnPoint.h"
#include "Item/ResourceItem/ResourceItemBase.h"
#include "Kismet/GameplayStatics.h"
#include "Framework/Manager/LCTimeManager.h"

AResourceItemSpawnManager::AResourceItemSpawnManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bSpawnOnBeginPlay = true;
}

void AResourceItemSpawnManager::BeginPlay()
{
	Super::BeginPlay();

	// 서버만 아이템 스폰
	if (HasAuthority() == true)
	{
		InitializeThemeItemMap();

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

	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			const auto Condition = Point->SpawnTimeCondition;

			// 해당 시간에만 스폰
			if ((NewPhase == EDayPhase::Night && Condition == ESpawnTimeCondition::NightOnly) ||
				(NewPhase == EDayPhase::Day && Condition == ESpawnTimeCondition::DayOnly))
			{
				Point->TrySpawnItem();
			}

			// 반대 시간대면 디스폰
			if ((NewPhase == EDayPhase::Night && Condition == ESpawnTimeCondition::DayOnly) ||
				(NewPhase == EDayPhase::Day && Condition == ESpawnTimeCondition::NightOnly))
			{
				if (Point->SpawnedResourceItem)
				{
					Point->SpawnedResourceItem->Destroy();
					Point->SpawnedResourceItem = nullptr;
				}
			}
		}
	}
}

void AResourceItemSpawnManager::InitializeThemeItemMap()
{
	ThemeItemMap.Empty();

	ThemeItemMap.Add("Cave", {
		"IronOre", "AdamantiumOre", "RedMercury"
		});

	ThemeItemMap.Add("Forest", {
		"HealingHerb", "LightBloom", "PoisonMushroom"
		});

	ThemeItemMap.Add("Ruins", {
		"AncientRuneStone", "RadiantFragment", "SealedMask"
		});
}

void AResourceItemSpawnManager::SpawnItemsForTheme()
{
	if (ThemeItemMap.Contains(CurrentThemeTag)==false)
	{
		UE_LOG(LogTemp, Warning, TEXT("Theme '%s' not found in ThemeItemMap."), *CurrentThemeTag.ToString());
		return;
	}

	TArray<AActor*> FoundPoints;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AResourceItemSpawnPoint::StaticClass(), FoundPoints);

	const TArray<FName>& ItemsToSpawn = ThemeItemMap[CurrentThemeTag];

	for (AActor* Actor : FoundPoints)
	{
		if (AResourceItemSpawnPoint* Point = Cast<AResourceItemSpawnPoint>(Actor))
		{
			Point->PossibleItems = ItemsToSpawn;
			Point->TrySpawnItem();
		}
	}
}
