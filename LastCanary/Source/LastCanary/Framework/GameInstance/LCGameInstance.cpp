#include "Framework/GameInstance/LCGameInstance.h"
#include "UI/Manager/LCUIManagerSettings.h"
#include "Engine/Engine.h"
#include "LastCanary.h"


ULCUIManagerSettings* ULCGameInstance::GetUIManagerSettings() const
{
	if (UIManagerSettings)
	{
		return UIManagerSettings;
	}
	else
	{
		//UE_LOG(LogTemp, Warning, TEXT("UIManagerSettings is not set in the GameInstance."));
		return nullptr;
	}
}

void ULCGameInstance::LoadMapData()
{
    if (MapDataTable == nullptr)
    {
       // LOG_Frame_WARNING(TEXT("MapDataTable is not assigned in USFGameInstanceSubsystem."));
        return;
    }

    static const FString ContextString(TEXT("Map Lookup"));
    TArray<FMapDataRow*> AllMaps;
    MapDataTable->GetAllRows(ContextString, AllMaps);

    for (FMapDataRow* MapDataRow : AllMaps)
    {
        if (MapDataRow)
        {
            MapDataRow->MapID = FCrc::StrCrc32(*MapDataRow->MapInfo.MapName.ToString());
            //MapDataRow->MapID = GetTypeHash(MapDataRow->MapInfo.MapName);
            LOG_Frame_WARNING(TEXT("Loaded map: %s"), *MapDataRow->MapInfo.MapName.ToString());
        }
    }
}

void ULCGameInstance::LoadItemData()
{
    if (ItemDataTable == nullptr)
    {
       // LOG_Frame_WARNING(TEXT("ItemDataTable is not assigned in ULCGameInstance."));
        return;
    }

    static const FString ContextString(TEXT("Item Lookup"));
    TArray<FItemDataRow*> AllItems;
    ItemDataTable->GetAllRows(ContextString, AllItems);

    for (FItemDataRow* ItemDataRow : AllItems)
    {
        if (ItemDataRow)
        {
            ItemDataRow->ItemID = FCrc::StrCrc32(*ItemDataRow->ItemName.ToString());

            // 예: 아이디 해시화 → 캐싱 용도
            // int32 HashedID = GetTypeHash(ItemDataRow->ItemName);
            // ItemDataRow->ItemID = HashedID;

            /*LOG_Frame_WARNING(TEXT("Loaded item: %s (Price: %d)"),
                *ItemDataRow->ItemName.ToString(),
                ItemDataRow->ItemPrice);*/
        }
    }

   // LOG_Frame_WARNING(TEXT("총 %d개의 상점 아이템을 로딩했습니다."), AllItems.Num());
}

// BluePrint에서 Create Session을 합니다.
void ULCGameInstance::CreateSession_Implementation(const FString& ServerName, int AmountOfSlots)
{
    UE_LOG(LogTemp, Warning, TEXT("CreateSession called in C++"));
}

void ULCGameInstance::Shutdown()
{
    Super::Shutdown();

}

void ULCGameInstance::LoadGunData()
{
    if (GunDataTable == nullptr)
    {
        // LOG_Frame_WARNING(TEXT("GunDataTable is not assigned in ULCGameInstance."));
        return;
    }

    static const FString ContextString(TEXT("Gun Lookup"));
    TArray<FGunDataRow*> AllGuns;
    GunDataTable->GetAllRows(ContextString, AllGuns);

    for (FGunDataRow* GunDataRow : AllGuns)
    {
        if (GunDataRow)
        {
            // 예: 아이디 해시화 → 캐싱 용도
            int32 HashedID = GetTypeHash(GunDataRow->GunName);
            GunDataRow->GunID = HashedID;
        }
    }
}
