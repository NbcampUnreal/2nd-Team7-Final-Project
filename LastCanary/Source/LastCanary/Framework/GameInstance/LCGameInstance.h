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

public:
	void Login();
	void HandleLoginCompleted(int32 LocalUserNum, bool bWasSuccessful, const FUniqueNetId& UserId, const FString& Error);
	FDelegateHandle LoginDelegateHandle;
	
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

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void CreateSession(const FString& ServerName, int AmountOfSlots);

	virtual void CreateSession_Implementation(const FString& ServerName, int AmountOfSlots);


	UFUNCTION(BlueprintCallable, Category = "Session")
	FString GetPlayerName() const;

	UFUNCTION(BlueprintCallable, Category = "Session")
	bool IsPlayerLoggedIn() const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Session")
	bool bIsCreateSession;

	UFUNCTION(BlueprintCallable)
	void Shutdown() override;

};
