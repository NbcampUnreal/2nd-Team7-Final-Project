#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "DataTable/MapDataRow.h"
#include "LCGameInstanceSubsystem.generated.h"

/**
 * 
 */
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

	ULCUIManager* GetUIManager() const;
	void ChangeLevelByMapName(const FName& MapName);
	void ChangeLevelByMapID(int32 MapID);
};
