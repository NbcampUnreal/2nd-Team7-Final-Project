#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "DataTable/MapDataRow.h"
#include "DataTable/ItemDataRow.h"
#include "AdvancedSessions/Classes/AdvancedFriendsGameInstance.h"
#include "LCGameInstance.generated.h"

class ULCUIManagerSettings;

UCLASS()
class LASTCANARY_API ULCGameInstance : public UAdvancedFriendsGameInstance
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

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void CreateSession(const FString& ServerName, int AmountOfSlots);

	virtual void CreateSession_Implementation(const FString& ServerName, int AmountOfSlots);

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsCreateSession;

	UFUNCTION(BlueprintCallable)
	void Shutdown() override;

};
