#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DataTable/MapDataRow.h"
#include "DataTable/ItemDataRow.h"
#include "LCGameInstance.generated.h"

/**
 * 
 */
class ULCUIManagerSettings;
UCLASS()
class LASTCANARY_API ULCGameInstance : public UGameInstance
{
	GENERATED_BODY()
	
private:
	UPROPERTY(EditAnywhere, Category = "UI")
	ULCUIManagerSettings* UIManagerSettings;

public:
	ULCUIManagerSettings* GetUIManagerSettings() const;

	FMapDataRow CurrentMapInfoRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* MapDataTable;
	void LoadMapData();

	FItemDataRow CurrentItemInfoRow;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* ItemDataTable;
	void LoadItemData();
};
