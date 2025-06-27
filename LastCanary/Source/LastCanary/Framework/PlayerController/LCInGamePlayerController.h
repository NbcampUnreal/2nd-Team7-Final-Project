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
	UFUNCTION(Client, Reliable)
	void Client_ShowLevelInfo(int32 MapId);
	void Client_ShowLevelInfo_Implementation(int32 MapId);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPopupLevelInfo> PopupLevelInfoClass;
	UPROPERTY()
	UPopupLevelInfo* PopupLevelInfoInstance;

	UFUNCTION(Client, Reliable)
	void Client_ShowLoseVideo();
	void Client_ShowLoseVideo_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UVideoPlayWidget> LoseWidgetClass;
	UVideoPlayWidget* LoseWidgetInstance;
	
	UFUNCTION(Client, Reliable)
	void Client_ShowEscapeGateVideo(UDataTable* CheckListTable);
	void Client_ShowEscapeGateVideo_Implementation(UDataTable* CheckListTable);

private:
	void StartCheckList(UDataTable* CheckListTable);

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UVideoPlayWidget> EscapeGateWidgetClass;
	UVideoPlayWidget* EscapeGateWidgetInstance;

	UFUNCTION(Client, Reliable)
	void Client_OnGameEnd();
	void Client_OnGameEnd_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_ShowResult();
	void Client_ShowResult_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_StartChecklist(AChecklistManager* ChecklistManager);
	void Client_StartChecklist_Implementation(AChecklistManager* ChecklistManager);

	UFUNCTION(Server, Reliable)
	void Server_RequestSubmitChecklist(const TArray<FChecklistQuestion>& PlayerAnswers);
	void Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers);

	UFUNCTION(Client, Reliable)
	void Client_NotifyResultReady(const FChecklistResultData& ResultData);
	void Client_NotifyResultReady_Implementation(const FChecklistResultData& ResultData);
	
	void SetSpectatingTarget(AActor* NewTarget);
};
