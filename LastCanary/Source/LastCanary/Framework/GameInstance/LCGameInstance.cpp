#include "Framework/GameInstance/LCGameInstance.h"
#include "UI/Manager/LCUIManagerSettings.h"

ULCUIManagerSettings* ULCGameInstance::GetUIManagerSettings() const
{
	if (UIManagerSettings)
	{
		return UIManagerSettings;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSettings is not set in the GameInstance."));
		return nullptr;
	}
}

void ULCGameInstance::LoadMapData()
{
    if (MapDataTable == nullptr)
    {
        UE_LOG(LogTemp, Warning, TEXT("MapDataTable is not assigned in USFGameInstanceSubsystem."));
        return;
    }

    static const FString ContextString(TEXT("Map Lookup"));
    TArray<FMapDataRow*> AllMaps;
    MapDataTable->GetAllRows(ContextString, AllMaps);

    for (FMapDataRow* MapDataRow : AllMaps)
    {
        if (MapDataRow)
        {
            MapDataRow->MapID = GetTypeHash(MapDataRow->MapInfo.MapName);
            UE_LOG(LogTemp, Log, TEXT("Loaded map: %s"), *MapDataRow->MapInfo.MapName.ToString());
        }
    }
}
