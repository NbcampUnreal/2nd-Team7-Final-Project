#include "UI/UIElement/ResultWidget.h"
#include "UI/UIObject/PlayerResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"
#include "Framework/Manager/ChecklistManager.h"

#include "LastCanary.h"

void UResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	PlayerResults = { PlayerResult1, PlayerResult2, PlayerResult3, PlayerResult4 };

	if (!GetOwningPlayer()->HasAuthority())
	{
		if (AcceptButton)
		{
			AcceptButton->SetIsEnabled(false);
			AcceptButton->SetRenderOpacity(0.f);
		}

		if (WaitHostText)
		{
			WaitHostText->SetRenderOpacity(1.f);
		}
	}

	if (AcceptButton)
	{
		AcceptButton->OnClicked.AddUniqueDynamic(this, &UResultWidget::OnAcceptClicked);
	}
}

void UResultWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (AcceptButton)
	{
		AcceptButton->OnClicked.RemoveDynamic(this, &UResultWidget::OnAcceptClicked);
	}
}

void UResultWidget::SetTotalResultData(const FTotalResultData& TotalResult)
{
	ClearPlayerResult();

	if (ResultHeader)
	{
		const FString HeaderText = FString::Printf(TEXT("[%d회차] 게이트 탐사 결과"), TotalResult.CurrentRound);
		ResultHeader->SetText(FText::FromString(HeaderText));
	}

	if (MVPText)
	{
		const FString Formatted = FString::Printf(TEXT("MVP : %s"), *TotalResult.MVPName);
		MVPText->SetText(FText::FromString(Formatted));
	}

	if (RankText)
	{
		const FString Formatted = FString::Printf(TEXT("Rank : %s"), *TotalResult.TotalRank);
		RankText->SetText(FText::FromString(Formatted));
	}

	if (TotalResourceText)
	{
		const FString Formatted = FString::Printf(TEXT("총 수입 : %d"), TotalResult.TotalReources);
		TotalResourceText->SetText(FText::FromString(Formatted));
	}

	if (PaymentText)
	{
		const FString Formatted = FString::Printf(TEXT("상납금 : %d"), TotalResult.Payment);
		PaymentText->SetText(FText::FromString(Formatted));
	}

	if (IncomeText)
	{
		IncomeGold = TotalResult.TotalReources - TotalResult.Payment;
		const FString Formatted = FString::Printf(TEXT("순 수익 : %d"), IncomeGold);
		IncomeText->SetText(FText::FromString(Formatted));
	}

	if (ExpText)
	{
		const FString Formatted = FString::Printf(TEXT("탐사 포인트 : %d"), TotalResult.TotalEXP);
		ExpText->SetText(FText::FromString(Formatted));
	}

	for (int i = 0; i < TotalResult.PlayersResult.Num(); i++)
	{
		if (PlayerResults[i])
		{
			PlayerResults[i]->SetChecklistResult(TotalResult.PlayersResult[i]);
			if (PlayerResults[i]->DetailButton)
			{
				PlayerResults[i]->DetailButton->SetIsEnabled(true);
			}
		}
	}
}


void UResultWidget::ClearPlayerResult()
{
	for (auto PlayerResult : PlayerResults)
	{
		PlayerResult->ClearPlayerResult();
		if (PlayerResult->DetailButton)
		{
			PlayerResult->DetailButton->SetIsEnabled(false);
		}
		
	}
}

void UResultWidget::OnAcceptClicked()
{
	APlayerController* PC = GetOwningPlayer();
	if (!PC)
	{
		LOG_Frame_WARNING(TEXT("ResultMenu - AcceptClicked → PlayerController 없음"));
		return;
	}

	// 클라이언트면 무시
	//if (PC->GetNetMode() != NM_ListenServer)
	if (PC->HasAuthority() == false)
	{
		LOG_Frame_WARNING(TEXT("ResultMenu - AcceptClicked → 클라이언트이므로 무시"));
		return;
	}

	LOG_Frame_WARNING(TEXT("ResultMenu - AcceptClicked → 호스트이므로 BaseCamp로 이동"));

	if (ULCGameManager* GameDataManager = GetGameInstance()->GetSubsystem<ULCGameManager>())
	{
		if (GameDataManager->IsGameEnd())
		{
			GameDataManager->EndGame();
			//ALCInGamePlayerController* LCPC = Cast<ALCInGamePlayerController>(PC);
			//if (LCPC)
			//{
			//	LCPC->GetUIManager()->ShowGameOverWidget();
			//}

		}
		else
		{
			// 게임 데이터 초기화
			GameDataManager->EndCurrentRound();
			GameDataManager->UpdateGold(FString::Printf(TEXT("게이트 탐사 완료!")), IncomeGold);
			LOG_Frame_WARNING(TEXT("ResultMenu - AcceptClicked → EndCurrentRound 호출"));

			// 호스트만 처리
			if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				const FName BaseCampMapName = TEXT("BaseCamp");
				const int32 BaseCampID = FCrc::StrCrc32(*BaseCampMapName.ToString());
				GISubsystem->ChangeLevelByMapID(BaseCampID);
			}
		}
	}
}
