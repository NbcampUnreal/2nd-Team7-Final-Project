#include "Framework/PlayerController/LCInGamePlayerController.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/Manager/LCCheatManager.h"

#include "UI/UIElement/ResultMenu.h"

#include "Engine/World.h"
#include "EngineUtils.h"

ALCInGamePlayerController::ALCInGamePlayerController()
{

}

void ALCInGamePlayerController::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

}

void ALCInGamePlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->SetUIContext(ELCUIContext::InGame);
		}
	}

}

void ALCInGamePlayerController::Client_OnGameEnd_Implementation()
{

}

void ALCInGamePlayerController::Client_ShowGameEndUI_Implementation()
{

}

void ALCInGamePlayerController::Server_MarkPlayerAsEscaped_Implementation()
{
	LOG_Frame_WARNING(TEXT("== Server_MarkPlayerAsEscaped_Implementation Called =="));

	if (GetWorld()->GetGameState<ALCGameState>())
	{
		GetWorld()->GetGameState<ALCGameState>()->MarkPlayerAsEscaped(PlayerState);
	}
}

void ALCInGamePlayerController::Client_StartChecklist_Implementation(AChecklistManager* ChecklistManager)
{
	LOG_Frame_WARNING(TEXT("로컬 컨트롤러가 되었고, 체크리스트를 띄울 준비를 하는 중"));
	if (ChecklistManager)
	{
		ChecklistManager->StartChecklist();
		LOG_Frame_WARNING(TEXT("체크리스트를 게임모드에서 받아서 띄움"));
	}
	else
	{
		LOG_Frame_WARNING(TEXT("체크리스트가 클라이언트에서 유효하지 않음"));
	}
}

void ALCInGamePlayerController::Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers)
{
	LOG_Frame_WARNING(TEXT("Server_RequestSubmitChecklist_Implementation called"));

	for (TActorIterator<AChecklistManager> It(GetWorld()); It; ++It)
	{
		if (AChecklistManager* Manager = *It)
		{
			LOG_Frame_WARNING(TEXT("ChecklistManager found → Submitting"));
			Manager->Server_SubmitChecklist(this, PlayerAnswers);
			return;
		}
	}

	LOG_Frame_WARNING(TEXT("ChecklistManager not found on server"));
}

void ALCInGamePlayerController::Client_NotifyResultReady_Implementation(const FChecklistResultData& ResultData)
{
	LOG_Frame_WARNING(TEXT("[Client] 결과 수신 → 결과 UI 출력 시작"));

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = GISubsystem->GetUIManager())
		{
			UIManager->ShowResultMenu();
			if (UResultMenu* Menu = UIManager->GetResultMenuClass())
			{
				Menu->SetChecklistResult(ResultData);
			}
			else
			{
				LOG_Frame_WARNING(TEXT("[Client] GetCachedResultMenu가 null을 반환함"));
			}
		}
	}
}
