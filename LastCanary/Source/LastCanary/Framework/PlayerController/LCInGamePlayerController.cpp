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


void ALCInGamePlayerController::Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers)
{
	LOG_Frame_WARNING(TEXT("Server_RequestSubmitChecklist_Implementation called"));

	for (TActorIterator<AChecklistManager> It(GetWorld()); It; ++It)
	{
		if (AChecklistManager* Manager = *It)
		{
			LOG_Frame_WARNING(TEXT("ChecklistManager found → Submitting"));
			Manager->SubmitCheckList(this, PlayerAnswers);
			//Manager->Server_SubmitChecklist(this, PlayerAnswers);
			//return;
		}
	}

	LOG_Frame_WARNING(TEXT("ChecklistManager not found on server"));

	if (ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>())
	{
		LCGM->SubmitChecklist(this, PlayerAnswers);
	}
}

void ALCInGamePlayerController::ClearResourceItem()
{
	ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}

	FItemDataRow* DefaultItem = Subsystem->GetItemDataByRowName("Default");
	if (DefaultItem == nullptr)
	{
		LOG_Server_ERROR(TEXT("Can't Find DefaultItem By RowName"));
	}

	ABasePlayerState* PS = Cast<ABasePlayerState>(PlayerState);
	if (PS)
	{
		//auto itemId = PS->AquiredItemIDs;
		for (int i = 0; i < PS->AquiredItemIDs.Num(); i++)
		{
			FItemDataRow* ItemData = Subsystem->GetItemDataByItemID(PS->AquiredItemIDs[i]);
			if (ItemData != nullptr)
			{
				if (ItemData->bIsResourceItem)
				{
					PS->AquiredItemIDs[i] = DefaultItem->ItemID;
				}
			}
		}
	}
}

void ALCInGamePlayerController::Server_ResetPlayerState_Implementation()
{
	ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!IsValid(Subsystem))
	{
		return;
	}

	FItemDataRow* DefaultItem = Subsystem->GetItemDataByRowName("Default");
	if (DefaultItem == nullptr)
	{
		LOG_Server_ERROR(TEXT("Can't Find DefaultItem By RowName"));
	}

	ABasePlayerState* PS = Cast<ABasePlayerState>(PlayerState);
	if (PS)
	{
		// Clear Resource Item
		for (int i = 0; i < PS->AquiredItemIDs.Num(); i++)
		{
			FItemDataRow* ItemData = Subsystem->GetItemDataByItemID(PS->AquiredItemIDs[i]);
			if (ItemData != nullptr)
			{
				if (ItemData->bIsResourceItem || ItemData->bIsNoteItem)
				{
					PS->AquiredItemIDs[i] = DefaultItem->ItemID;
					continue;
				}
			}
		}

		PS->KillCount = 0;
		PS->SurviveTime = 0;
	}

	// TO DO : 단서관련(노트) 아이템들도 초기화

}

void ALCInGamePlayerController::Client_ShowResultWidget_Implementation(const FTotalResultData& ResultData)
{
	if (LCUIManager)
	{
		UResultWidget* ResultWidget = LCUIManager->ShowResultWidget();
		if (ResultWidget)
		{
			ResultWidget->SetTotalResultData(ResultData);
			Server_ResetPlayerState();
		}
	}
}
