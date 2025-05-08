#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Kismet/GameplayStatics.h"

void ULCGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LCUIManager = NewObject<ULCUIManager>(GetGameInstance());
	ULCGameInstance* GI = Cast<ULCGameInstance>(GetGameInstance());
	if (GI)
	{
		MapDataTable = GI->MapDataTable;
		GI->LoadMapData();
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
		UE_LOG(LogTemp, Warning, TEXT("MapDataTable is not assigned."));
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
			UE_LOG(LogTemp, Warning, TEXT("Trying to open level: %s"), *MapRow->MapInfo.MapName.ToString());
			return;
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("Map not found for ID: %d"), MapID);
}