#pragma once

#include "CoreMinimal.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Framework/GameInstance/LCGameManager.h"
#include "Character/BasePlayerController.h"
#include "LCInGamePlayerController.generated.h"

class AChecklistManager;
class UPopupLevelInfo;
class UInputAction;
class UVideoPlayWidget;
class ULoseWidget;
UCLASS()
class LASTCANARY_API ALCInGamePlayerController : public ABasePlayerController
{
	GENERATED_BODY()

protected:
	ALCInGamePlayerController();

	virtual void PostSeamlessTravel() override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPopupLevelInfo> PopupLevelInfoClass;
	UPROPERTY()
	UPopupLevelInfo* PopupLevelInfoInstance;

	UFUNCTION(Client, Reliable)
	void Client_ShowLevelInfo(int32 MapId);
	void Client_ShowLevelInfo_Implementation(int32 MapId);

	UFUNCTION(Client, Reliable)
	void Client_OnEscapeGate(UDataTable* CheckListTable);
	void Client_OnEscapeGate_Implementation(UDataTable* CheckListTable);
	UFUNCTION(Client, Reliable)
	void Client_OnGameLose();
	void Client_OnGameLose_Implementation();
	UFUNCTION(Client, Reliable)
	void Client_OnGameEnd();
	void Client_OnGameEnd_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UVideoPlayWidget> LoseWidgetClass;
	UVideoPlayWidget* LoseWidgetInstance;
	void ShowLoseVideo();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UVideoPlayWidget> EscapeGateWidgetClass;
	UVideoPlayWidget* EscapeGateWidgetInstance;
	void ShowEscapeGateVideo(UDataTable* CheckListTable);

private:
	void StartCheckList(UDataTable* CheckListTable);

public:
	UFUNCTION(Server, Reliable)
	void Server_RequestSubmitChecklist(const TArray<FChecklistQuestion>& PlayerAnswers);
	void Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers);

	UFUNCTION(Client, Reliable)
	void Client_ShowResultWidget(const FTotalResultData& ResultData);
	void Client_ShowResultWidget_Implementation(const FTotalResultData& ResultData);

	UFUNCTION(Server, Reliable)
	void Server_ResetPlayerState();
	void Server_ResetPlayerState_Implementation();

	void ClearResourceItem();
};
