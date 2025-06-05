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

	virtual void ClientReturnToMainMenuWithTextReason_Implementation(const FText& ReturnReason) override;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void ClientWasKicked_Implementation(const FText& KickReason);
	void ClientWasKicked_Implementation_Implementation(const FText& KickReason);

	void StartGame(FString SoftPath);

protected:
	TObjectPtr<ULCUIManager> LCUIManager;
};
