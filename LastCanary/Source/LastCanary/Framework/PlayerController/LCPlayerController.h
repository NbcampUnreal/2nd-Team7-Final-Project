#pragma once

#include "CoreMinimal.h"
#include "UI/Manager/LCUIManager.h"
#include "DataType/SessionPlayerInfo.h"
#include "GameFramework/PlayerController.h"
#include "LCPlayerController.generated.h"

class ULCUIManager;

UCLASS()
class LASTCANARY_API ALCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	ALCPlayerController();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void Server_SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo);
	virtual void Server_SetPlayerInfo_Implementation(const FSessionPlayerInfo& PlayerInfo);

	UFUNCTION(Client, Reliable)
	void Client_UpdatePlayerList(const TArray<FSessionPlayerInfo>& PlayerInfos);
	virtual void Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos);

	UFUNCTION(Client, Reliable)
	void Client_ShowLoading();
	void Client_ShowLoading_Implementation();

	UFUNCTION(Client, Unreliable)
	void Client_ReceiveMessageFromGM(const FString& Message);
	void Client_ReceiveMessageFromGM_Implementation(const FString& Message);

	// 서버에서 플레이어 강퇴시 호출 됨
	void ClientWasKicked_Implementation(const FText& KickReason);
	// 강퇴시나 PreLogIn에서 ErrorMessage가 있으면 해당 함수를 통해 
	// MainMenu로 돌아가는거 같은데 디버깅이 안됨... 아오...
	void ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason);

	void StartGame(FString SoftPath);

protected:
	TObjectPtr<ULCUIManager> LCUIManager;
};
