#include "UI/UIObject/DetailResultWidget.h"

#include "Components/TextBlock.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "UI/UIObject/ResourceScoreEntry.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"
#include "DataTable/ItemDataRow.h"

#include "LastCanary.h"

void UDetailResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UDetailResultWidget::OnExitButtonClicked);
	}

}

void UDetailResultWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UDetailResultWidget::OnExitButtonClicked);
	}
}

void UDetailResultWidget::SetChecklistResult(const FPlayerResultData& ResultData)
{
	//ClearPlayerResult();
	if (PlayerNameText && ResultData.OwnerController)
	{
		const FString PlayerName = FString::Printf(TEXT("Player : %s"), *ResultData.OwnerController->PlayerState->GetPlayerName());
		PlayerNameText->SetText(FText::FromString(PlayerName));
	}
	if (SurvivalText)
	{
		SurvivalText->SetText(ResultData.bIsSurvived ? FText::FromString(TEXT("Survive")) : FText::FromString(TEXT("Death")));
	}
	if (SurviveTimeText)
	{
		int32 SurviveTime = ResultData.SurviveTime;

		int32 Minutes = SurviveTime / 60;
		int32 Seconds = SurviveTime % 60;

		const FString Formatted = FString::Printf(TEXT("Survive Time : %02dm %02ds"), Minutes, Seconds);
		SurviveTimeText->SetText(FText::FromString(Formatted));
	}
	if (AccuracyText)
	{
		const FString Formatted = FString::Printf(TEXT("Accuracy : %.f%%"), ResultData.CorrectRate * 100);
		AccuracyText->SetText(FText::FromString(Formatted));
	}
	if (KillText)
	{
		const FString Formatted = FString::Printf(TEXT("Kill Count : %d"), ResultData.KillCount);
		KillText->SetText(FText::FromString(Formatted));
	}


	if (SurvivalPointText)
	{
		int32 Point = ResultData.bIsSurvived ? 50 : 0;
		FString SurviveText = FString::Printf(TEXT("Survive Point : %d"), Point);
		SurvivalPointText->SetText(FText::FromString(SurviveText));
	}
	if (SurviveTimePointText)
	{
		const FString Formatted = FString::Printf(TEXT("Survive Time Point : %.f"), ResultData.SurviveTime * 0.5f);
		SurviveTimePointText->SetText(FText::FromString(Formatted));
	}
	if (AccuracyPointText)
	{
		const FString Formatted = FString::Printf(TEXT("Accuracy Point : %.f"), ResultData.CorrectRate * 100);
		AccuracyPointText->SetText(FText::FromString(Formatted));
	}
	if (KillPointText)
	{
		const FString Formatted = FString::Printf(TEXT("Kill Point : %d"), ResultData.KillCount * 20);
		KillPointText->SetText(FText::FromString(Formatted));
	}


	if (ClueText)
	{
		const FString Formatted = FString::Printf(TEXT("Gathered Clues : %d"), ResultData.ExplorePointDetails.Num());
		ClueText->SetText(FText::FromString(Formatted));

		//TO DO : 단서에 따른 탐사포인트 결산 내역 보여주기
	}

	TArray<FExplorePointInfo> CorrectClues;
	TArray<FExplorePointInfo> LieClues;
	TArray<FExplorePointInfo> NoiseClues;

	for (auto Clue : ResultData.ExplorePointDetails)
	{
		ENoteType NoteType = static_cast<ENoteType>(Clue.NoteTypeIndex);
		switch (NoteType)
		{
		case ENoteType::Truth:
			CorrectClues.Add(Clue);
			break;
		case ENoteType::Lie:
			LieClues.Add(Clue);
			break;
		case ENoteType::Noise:
		default:
			NoiseClues.Add(Clue);
			break;
		}
	}

	if (CorrectClueText)
	{
		int32 ClueNum = CorrectClues.Num();
		int32 Point = 50;
		FString CluePoint = FString::Printf(TEXT("- Correct Clue Point : %dea * %d = %d"), ClueNum, Point, ClueNum * Point);
		CorrectClueText->SetText(FText::FromString(CluePoint));
	}
	if (WorngClueText)
	{
		int32 ClueNum = LieClues.Num();
		int32 Point = -20;
		FString CluePoint = FString::Printf(TEXT("- Worng Clue Point : %dea * %d = %d"), ClueNum, Point, ClueNum * Point);
		WorngClueText->SetText(FText::FromString(CluePoint));
	}
	if (NoiseClueText)
	{
		int32 ClueNum = NoiseClues.Num();
		int32 Point = 0;
		FString CluePoint = FString::Printf(TEXT("- Noise Clue Point : %dea * %d = %d"), ClueNum, Point, ClueNum * Point);
		NoiseClueText->SetText(FText::FromString(CluePoint));
	}


	if (ExpText)
	{
		const FString Formatted = FString::Printf(TEXT("Exp : %d"), ResultData.ExplorePoint);
		ExpText->SetText(FText::FromString(Formatted));
	}


	if (ResourceText)
	{
		const FString Formatted = FString::Printf(TEXT("Resrouce Score : %d"), ResultData.ResourcePoint);
		ResourceText->SetText(FText::FromString(Formatted));
	}

	//if (ScoreText)
	//{
	//	const FString Formatted = FString::Printf(TEXT("Total Score : %d"), ResultData.TotalScore);
	//	ScoreText->SetText(FText::FromString(Formatted));
	//}

	if (RankText)
	{
		const FString Formatted = FString::Printf(TEXT("Rank : %s"), *ResultData.Rank);
		RankText->SetText(FText::FromString(Formatted));
	}
	

	if (GatheredResourceText)
	{
		GatheredResourceText->SetText(FText::FromString("Gathered Resources"));
	}

	CachedResourceDetails = ResultData.ResourceDetails;
	CurrentEntryIndex = 0;
	CurrentResourceIndex = 0;
	bIsAddingResources = false;

	if (RewardScrollBox == nullptr)
	{
		return;
	}

	RewardScrollBox->ClearChildren();

	GetWorld()->GetTimerManager().SetTimer(
		EntryAddTimerHandle,
		this,
		&UDetailResultWidget::AddNextEntry, // 이름 통일
		0.3f,
		true
	);
}

void UDetailResultWidget::AddNextEntry()
{
	if (!RewardScrollBox)
	{
		GetWorld()->GetTimerManager().ClearTimer(EntryAddTimerHandle);
		return;
	}

	// 자원 점수 항목 추가 단계
	if (CachedResourceDetails.IsValidIndex(CurrentResourceIndex))
	{
		if (ResourceEntryClass)
		{
			const FResourceScoreInfo& Info = CachedResourceDetails[CurrentResourceIndex];

			UResourceScoreEntry* Entry = CreateWidget<UResourceScoreEntry>(this, ResourceEntryClass);
			if (Entry)
			{
				const FString Breakdown = FString::Printf(
					TEXT("%d × %.f × %d = %d"),
					Info.BaseScore,
					Info.Multiplier,
					Info.Amount,
					Info.TotalScore);

				Entry->InitResourceScoreEntry(FText::FromName(Info.ResourceID), FText::FromString(Breakdown));
				RewardScrollBox->AddChild(Entry);
			}
		}

		CurrentResourceIndex++;
		return;
	}

	// 모든 출력 완료 → 타이머 종료
	GetWorld()->GetTimerManager().ClearTimer(EntryAddTimerHandle);

	LOG_Frame_WARNING(TEXT("ResultMenu - ResourceScore Entries Displayed: %d"), CachedResourceDetails.Num());

	//if (FadeInRankAnim && RankText)
	//{
	//	PlayAnimation(FadeInRankAnim);
	//	LOG_Frame_WARNING(TEXT("ResultMenu - RankText FadeIn 애니메이션 실행"));
	//}
}

void UDetailResultWidget::OnExitButtonClicked()
{
	RemoveFromParent();

}
