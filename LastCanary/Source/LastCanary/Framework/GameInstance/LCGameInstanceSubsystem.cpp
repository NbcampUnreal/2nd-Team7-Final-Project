#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/GameUserSettings.h"

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
		GunDataTable = GI->GunDataTable;
		GI->LoadMapData();
		GI->LoadItemData();
		GI->LoadGunData();
	}


#if WITH_EDITOR
	if (GEngine)
	{
		UWorld* World = GetWorld();
		if (World)
		{
			GEngine->Exec(World, TEXT("net.AllowPIESeamlessTravel 1"));
			UE_LOG(LogTemp, Warning, TEXT("[LCGameInstanceSubsystem] PIE SeamlessTravel enabled."));
		}
	}
#endif

	//세이브 데이터 불러오기
	LoadSaveData();
}

ULCUIManager* ULCGameInstanceSubsystem::GetUIManager() const
{
	return LCUIManager;
}

void ULCGameInstanceSubsystem::ChangeLevelByMapName(const FName& MapName)
{
	const int32 MapID = FCrc::StrCrc32(*MapName.ToString());
	// const int32 MapID = GetTypeHash(MapName);
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
			// UWorld 경로 → ServerTravel용 문자열로 변환
			const FString AssetPath = MapRow->MapInfo.MapPath.ToSoftObjectPath().ToString(); // "/Game/Maps/MapName.MapName"
			const FString CleanedPath = FPackageName::ObjectPathToPackageName(AssetPath);   // "/Game/Maps/MapName"
			const FString TravelURL = FString::Printf(TEXT("%s?listen"), *CleanedPath);

			// SeamlessTravel 활성화
			if (AGameModeBase* GM = GetWorld()->GetAuthGameMode())
			{
				GM->bUseSeamlessTravel = true;
			}

			LOG_Frame_WARNING(TEXT("Trying to ServerTravel to: %s (MapID: %d)"), *TravelURL, MapID);
			GetWorld()->ServerTravel(TravelURL, true);
			return;
		}
	}

	LOG_Frame_WARNING(TEXT("Map not found for ID: %d"), MapID);
}


FItemDataRow* ULCGameInstanceSubsystem::GetItemDataByRowName(FName ItemRowName) const
{
	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[ULCGameInstanceSubsystem::GetItemDataByRowName] ItemDataTable이 없습니다."));
		return nullptr;
	}

	return ItemDataTable->FindRow<FItemDataRow>(ItemRowName, TEXT("GetItemDataByRowName"));
}

UDataTable* ULCGameInstanceSubsystem::GetMapDataTable() const
{
	return MapDataTable;
}

UDataTable* ULCGameInstanceSubsystem::GetItemDataTable() const
{
	return ItemDataTable;
}

UDataTable* ULCGameInstanceSubsystem::GetGunDataTable() const
{
	return GunDataTable;
}

void ULCGameInstanceSubsystem::LoadSaveData()
{
	if (!IsValid(GetUIManager()))
	{
		LOG_Char_WARNING(TEXT("UIManager가 유효하지 않음"));		
		return;
	}
	else
	{
		LOG_Char_WARNING(TEXT("UIManager가 유효함"));
	}

	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		
		LOG_Char_WARNING(TEXT("Settings->LoadSettings(true)"));
		Settings->LoadSettings(true);
		Settings->ApplySettings(false);	
	}
}