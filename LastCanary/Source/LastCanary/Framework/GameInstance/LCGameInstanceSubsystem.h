#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTable/MapDataRow.h"
#include "DataTable/ItemDataRow.h"
#include "LCGameInstanceSubsystem.generated.h"

class ULCLocalPlayerSaveGame;
class ULCUIManager;

UCLASS()
class LASTCANARY_API ULCGameInstanceSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
private:
	UPROPERTY()
	ULCUIManager* LCUIManager;

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Map Data")
	TObjectPtr<UDataTable> MapDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<UDataTable> ItemDataTable;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<UDataTable> GunDataTable;

	UFUNCTION(BlueprintCallable)
	ULCUIManager* GetUIManager() const;
	void ChangeLevelByMapName(const FName& MapName);
	void ChangeLevelByMapID(int32 MapID);

	FItemDataRow* GetItemDataByRowName(FName ItemRowName) const;

	UFUNCTION(BlueprintPure, Category = "Data")
	UDataTable* GetMapDataTable() const;
	
	UFUNCTION(BlueprintPure, Category = "Data")
	UDataTable* GetItemDataTable() const;

	UFUNCTION(BlueprintPure, Category = "Data")
	UDataTable* GetGunDataTable() const;



	void LoadSaveData();

	void LoadUserSettings();
//public:
//	int32 UserNum = 0;
//public:
//	void SetUserNum(int NumOfUsers)
//	{
//		UserNum = NumOfUsers;
//	}
};
