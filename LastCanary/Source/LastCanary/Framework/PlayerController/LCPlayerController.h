#pragma once

#include "CoreMinimal.h"
#include "UI/Manager/LCUIManager.h"
#include "DataType/SessionPlayerInfo.h"
#include "LastCanary.h"
#include "Framework/PlayerController/LCPlayerInputController.h"
#include "Character/CinematicDummyCharacter.h"
#include "LevelSequenceActor.h"
#include "Framework/Manager/GateCutsceneManager.h"
#include "LCPlayerController.generated.h"

class ULCUIManager;
class ALCGateActor;
class ACineCameraActor;
class ULevelSequence;
class ABaseCharacter;
struct FCharacterCustomizationData;

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

	 void ToggleShowRoomWidget() override;

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
	virtual void Client_ReceiveMessageFromGM_Implementation(const FString& Message);

public:
	void StartGame(FString SoftPath);

	UPROPERTY(EditAnywhere)
	ACineCameraActor* SequenceCamera;
	UPROPERTY(EditInstanceOnly, Category = "Cutscene")
	ALCGateActor* LinkedGateActor;

	UFUNCTION()
	void SetLinkedGateActor(ALCGateActor* InGateActor) { LinkedGateActor = InGateActor; }
	UFUNCTION(BlueprintCallable)
	ALCGateActor* GetLinkedGateActor() const { return LinkedGateActor; }

	UFUNCTION(Client, Reliable)
	void Client_HideHUD();
	void Client_HideHUD_Implementation();

	UFUNCTION(Client, Unreliable)
	void Client_PlayGateCutscene(ULevelSequence* Sequence, ACinematicDummyCharacter* CinematicDummyCharacter, const FTransform& SpawnTransform, int32 PlayerIndex, ECutsceneType CutsceneType);
	void Client_PlayGateCutscene_Implementation(ULevelSequence* Sequence, ACinematicDummyCharacter* CinematicDummyCharacter, const FTransform& SpawnTransform, int32 PlayerIndex, ECutsceneType CutsceneType);

	// LevelSequenceActor를 클라이언트에 알려주기 위한 변수
	UPROPERTY(Replicated)
	ALevelSequenceActor* LinkedSequenceActor;

	UFUNCTION()
	void OnCutsceneFinished();

	// 헬퍼 함수들 추가
	void SetCameraFromSequence(ALevelSequenceActor* SequenceActor);
	void HideUIForCutscene();
	void ShowUIAfterCutscene();

	// 현재 컷신 타입 저장용 변수
	UPROPERTY()
	ECutsceneType CurrentCutsceneType;

	UFUNCTION(Server, Reliable)
	void Server_RequestIntoGameLevel();
	void Server_RequestIntoGameLevel_Implementation();

	// 레벨 이동 요청 (베이스로 돌아가기)
	UFUNCTION(Server, Reliable)
	void Server_RequestReturnToBase();
	void Server_RequestReturnToBase_Implementation();

	ULCUIManager* GetUIManager() { return LCUIManager; }
protected:
	TObjectPtr<ULCUIManager> LCUIManager;
};
