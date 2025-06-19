#pragma once

#include "CoreMinimal.h"
#include "UI/Manager/LCUIManager.h"
#include "DataType/SessionPlayerInfo.h"
#include "LastCanary.h"
#include "Framework/PlayerController/LCPlayerInputController.h"
#include "LCPlayerController.generated.h"

class ULCUIManager;

UCLASS()
class LASTCANARY_API ALCPlayerController : public ALCPlayerInputController
{
	GENERATED_BODY()
	
public:
	ALCPlayerController();

	virtual void PostSeamlessTravel() override;
	virtual void BeginPlay() override;

	void TryRestoreInventory();
	void DelayedPostTravelSetup();

public:

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo);
	virtual void Server_SetPlayerInfo_Implementation(const FSessionPlayerInfo& PlayerInfo);

	UFUNCTION(Client, Reliable)
	void Client_UpdatePlayerList(const TArray<FSessionPlayerInfo>& PlayerInfos);
	virtual void Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos);

	void UpdatePlayerList(const TArray<FSessionPlayerInfo>& PlayerInfos);

	// 서버에서 플레이어 강퇴시 호출 됨
	void ClientWasKicked_Implementation(const FText& KickReason);
	// 강퇴시나 PreLogIn에서 ErrorMessage가 있으면 해당 함수를 통해 
	// MainMenu로 돌아가는거 같은데 디버깅이 안됨... 아오...
	void ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason);

	virtual void ToggleShowRoomWidget() override;

private:
	FTimerHandle UpdatePlayerListTimerHandle;
	float RePeatRate = 0.3f;

public:
	UFUNCTION(Client, Reliable)
	void Client_ShowLoading();
	void Client_ShowLoading_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_HideLoading();
	void Client_HideLoading_Implementation();

	UFUNCTION(Client, Unreliable)
	void Client_ReceiveMessageFromGM(const FString& Message);
	void Client_ReceiveMessageFromGM_Implementation(const FString& Message);

public:
	void StartGame(FString SoftPath);

	ULCUIManager* GetUIManager() { return LCUIManager; }
protected:
	TObjectPtr<ULCUIManager> LCUIManager;
};
