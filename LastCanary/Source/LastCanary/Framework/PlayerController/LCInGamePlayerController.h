#pragma once

#include "CoreMinimal.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Character/BasePlayerController.h"
#include "LCInGamePlayerController.generated.h"

class AChecklistManager;

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
	void Client_NotifyResultReady(const FChecklistResultData& ResultData);
	void Client_NotifyResultReady_Implementation(const FChecklistResultData& ResultData);

	UFUNCTION(Client, Reliable)
	void Client_StartChecklist(AChecklistManager* ChecklistManager);
	void Client_StartChecklist_Implementation(AChecklistManager* ChecklistManager);

	UFUNCTION(Server, Reliable)
	void Server_MarkPlayerAsEscaped();
	void Server_MarkPlayerAsEscaped_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_RequestSubmitChecklist(const TArray<FChecklistQuestion>& PlayerAnswers);
	void Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers);
};
