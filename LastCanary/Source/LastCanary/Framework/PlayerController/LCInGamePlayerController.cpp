#include "Framework/PlayerController/LCInGamePlayerController.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/Manager/LCCheatManager.h"
#include "Framework/GameMode/LCInGameModeBase.h"
#include "Character/BasePlayerState.h"

#include "UI/UIElement/ResultMenu.h"
#include "UI/UIElement/ResultWidget.h"
#include "UI/Popup/PopupLevelInfo.h"
#include "UI/UIElement/VideoPlayWidget.h"

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

	if (IsLocalController())
	{
		if (PopupLevelInfoClass)
		{
			PopupLevelInfoInstance = CreateWidget<UPopupLevelInfo>(this, PopupLevelInfoClass);
		}
	}

	if (LCUIManager)
	{
		LCUIManager->ShowLoadingLevel();
	}

}

void ALCInGamePlayerController::Client_ShowLevelInfo_Implementation(int32 MapId)
{
	if (PopupLevelInfoInstance)
	{
		PopupLevelInfoInstance->AddToViewport();

		if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			FMapDataRow* MapData = Subsystem->GetMapDataByMapID(MapId);
			PopupLevelInfoInstance->InitLevelInfoWidget(MapData);
		}
	}
}


void ALCInGamePlayerController::Client_OnGameEnd_Implementation()
{
	LCUIManager->ShowGameEndWidget();
}

void ALCInGamePlayerController::Client_ShowResult_Implementation()
{

	LCUIManager->ShowResultMenu();
}

void ALCInGamePlayerController::Client_ShowLoseVideo_Implementation()
{
	if (LoseWidgetClass && !LoseWidgetInstance)
	{
		LoseWidgetInstance = CreateWidget<UVideoPlayWidget>(this, LoseWidgetClass);
		if (LoseWidgetInstance)
		{
			LoseWidgetInstance->AddToViewport(100);

			LoseWidgetInstance->OnVideoEnded.BindLambda
			(
				[this]()
				{
					LOG_Frame_WARNING(TEXT("On Video Play Finished!!"));
					LoseWidgetInstance = nullptr;

					LCUIManager->ShowGameOverWidget();
				}
			);
		}
	}
}

void ALCInGamePlayerController::Client_ShowEscapeGateVideo_Implementation(UDataTable* CheckListTable)
{
	//this->StartCheckList(CheckListTable);
	if (EscapeGateWidgetClass && !EscapeGateWidgetInstance)
	{
		EscapeGateWidgetInstance = CreateWidget<UVideoPlayWidget>(this, EscapeGateWidgetClass);
		if (EscapeGateWidgetInstance)
		{
			EscapeGateWidgetInstance->AddToViewport(100);

			EscapeGateWidgetInstance->OnVideoEnded.BindLambda
			(
				[this, CheckListTable]()
				{
					LOG_Frame_WARNING(TEXT("On Video Play Finished!!"));
					EscapeGateWidgetInstance = nullptr;

					this->StartCheckList(CheckListTable);
				}
			);
		}
	}
}

void ALCInGamePlayerController::StartCheckList(UDataTable* CheckListTable)
{
	LOG_Frame_WARNING(TEXT("Start New CheckList : %s"), *this->PlayerState->GetPlayerName());
	LCUIManager->ShowNewChecklistWidget(CheckListTable);
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
