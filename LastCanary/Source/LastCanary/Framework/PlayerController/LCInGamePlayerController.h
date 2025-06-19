#pragma once

#include "CoreMinimal.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Character/BasePlayerController.h"
#include "InputActionValue.h" // For FInputActionInstance
#include "EnhancedInputComponent.h" // For UEnhancedInputComponent
#include "InputTriggers.h" // For ETriggerEvent
#include "LCInGamePlayerController.generated.h"

class AChecklistManager;
class UInputAction;
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
	void Client_OnGameEnd();
	void Client_OnGameEnd_Implementation();
	
	UFUNCTION(Client, Reliable)
	void Client_ShowGameEndUI();
	void Client_ShowGameEndUI_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_MarkPlayerAsEscaped();
	void Server_MarkPlayerAsEscaped_Implementation();

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
