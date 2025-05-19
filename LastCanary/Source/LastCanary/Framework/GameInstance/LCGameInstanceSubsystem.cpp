#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

void ULCGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LCUIManager = NewObject<ULCUIManager>(this, ULCUIManager::StaticClass());
	ULCGameInstance* GI = Cast<ULCGameInstance>(GetGameInstance());
	if (GI)
	{
		MapDataTable = GI->MapDataTable;
		ItemDataTable = GI->ItemDataTable;
		GI->LoadMapData();
		GI->LoadItemData();
	}
}

ULCUIManager* ULCGameInstanceSubsystem::GetUIManager() const
{
	return LCUIManager;
}

void ULCGameInstanceSubsystem::ChangeLevelByMapName(const FName& MapName)
{
	const int32 MapID = GetTypeHash(MapName);
	ChangeLevelByMapID(MapID);
}

void ULCGameInstanceSubsystem::ChangeLevelByMapID(int32 MapID)
{
	if (MapDataTable == nullptr)
	{
		LOG_Frame_WARNING(TEXT("MapDataTable is not assigned."));
		return;
	}

	static const FString ContextString(TEXT("MapInfo Context"));
	TArray<FMapDataRow*> AllMaps;
	MapDataTable->GetAllRows<FMapDataRow>(ContextString, AllMaps);

	for (FMapDataRow* MapRow : AllMaps)
	{
		if (MapRow && MapRow->MapID == MapID)
		{
			UGameplayStatics::OpenLevel(GetWorld(), MapRow->MapInfo.MapName);
			LOG_Frame_WARNING(TEXT("Trying to open level: %s"), *MapRow->MapInfo.MapName.ToString());
			return;
		}
	}

	LOG_Frame_WARNING(TEXT("Map not found for ID: %d"), MapID);
}