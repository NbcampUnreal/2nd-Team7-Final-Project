#include "UI/UIObject/PlayerResultWidget.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "UI/UIObject/DetailResultWidget.h"
#include "UI/UIObject/ResourceScoreEntry.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/PlayerState.h"

#include "LastCanary.h"

void UPlayerResultWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (DetailButton)
	{
		DetailButton->OnClicked.AddUniqueDynamic(this, &UPlayerResultWidget::OnDetailButtonClicked);
	}
}

void UPlayerResultWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (DetailButton)
	{
		DetailButton->OnClicked.RemoveDynamic(this, &UPlayerResultWidget::OnDetailButtonClicked);
	}
}

void UPlayerResultWidget::OnDetailButtonClicked()
{
	if (DetailWidgetClass)
	{
		UDetailResultWidget* DetailWidget = CreateWidget<UDetailResultWidget>(this, DetailWidgetClass);
		if (DetailWidget)
		{
			DetailWidget->AddToViewport(999);
			DetailWidget->SetChecklistResult(CachedResultData);
		}
	}
}

void UPlayerResultWidget::SetChecklistResult(const FPlayerResultData& ResultData)
{
	CachedResultData = ResultData;

	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::FromString(ResultData.PlayerName));
	}

	if (SurvivalText)
	{
		SurvivalText->SetText(ResultData.bIsSurvived ? FText::FromString(TEXT("Survive")) : FText::FromString(TEXT("Death")));
	}

	if (AccuracyText)
	{
		const FString Formatted = FString::Printf(TEXT("Accuracy : %.f%%"), ResultData.CorrectRate * 100);
		AccuracyText->SetText(FText::FromString(Formatted));
	}

	if (RankText)
	{
		const FString Formatted = FString::Printf(TEXT("Rank : %s"), *ResultData.Rank);
		RankText->SetText(FText::FromString(Formatted));
	}

	if (ExpText)
	{
		const FString Formatted = FString::Printf(TEXT("Exp : %d"), ResultData.ExplorePoint);
		ExpText->SetText(FText::FromString(Formatted));
	}

	if (ResourceText)
	{
		const FString Formatted = FString::Printf(TEXT("Resource : %d"), ResultData.ResourcePoint);
		ResourceText->SetText(FText::FromString(Formatted));
	}

	//if (SurviveTimeText)
	//{
	//	FString TestTime = TEXT("15M 15S");
	//	const FString Formatted = FString::Printf(TEXT("Survive Time : %s"), *TestTime);
	//	SurviveTimeText->SetText(FText::FromString(Formatted));
	//}

	//if (KillText)
	//{
	//	const FString Formatted = FString::Printf(TEXT("Kill Count : %d"), 15);
	//	KillText->SetText(FText::FromString(Formatted));
	//}

	//if (ClueText)
	//{
	//	const FString Formatted = FString::Printf(TEXT("Gathered Clues : %d"), 8);
	//	ClueText->SetText(FText::FromString(Formatted));
	//}

	//if (GatheredResourceText)
	//{
	//	GatheredResourceText->SetText(FText::FromString("Gathered Resources"));
	//}

	//CachedResourceDetails = ResultData.ResourceDetails;
	//CurrentEntryIndex = 0;
	//CurrentResourceIndex = 0;
	//bIsAddingResources = false;

	//if (RewardScrollBox == nullptr)
	//{
	//	return;
	//}

	//RewardScrollBox->ClearChildren();

	//GetWorld()->GetTimerManager().SetTimer(
	//	EntryAddTimerHandle,
	//	this,
	//	&UPlayerResultWidget::AddNextEntry, // 이름 통일
	//	0.3f,
	//	true
	//);

	//if (RewardScrollBox && ResourceEntryClass)
	//{
	//	RewardScrollBox->ClearChildren();
	//	for (const auto& Resource : ResultData.ResourceDetails)
	//	{
	//		if (UResourceScoreEntry* Entry = CreateWidget<UResourceScoreEntry>(this, ResourceEntryClass))
	//		{
	//			RewardScrollBox->AddChild(Entry);
	//		}
	//	}
	//}
}

void UPlayerResultWidget::ClearPlayerResult()
{
	if (PlayerNameText)
	{
		PlayerNameText->SetText(FText::GetEmpty());
	}
	if (SurvivalText)
	{
		SurvivalText->SetText(FText::GetEmpty());
	}
	if (AccuracyText)
	{
		AccuracyText->SetText(FText::GetEmpty());
	}
	//if (SurviveTimeText)
	//{
	//	SurviveTimeText->SetText(FText::GetEmpty());
	//}
	//if (KillText)
	//{
	//	KillText->SetText(FText::GetEmpty());
	//}
	if (ExpText)
	{
		ExpText->SetText(FText::GetEmpty());
	}
	//if (ClueText)
	//{
	//	ClueText->SetText(FText::GetEmpty());
	//}
	if (ResourceText)
	{
		ResourceText->SetText(FText::GetEmpty());
	}
	//if (GatheredResourceText)
	//{
	//	GatheredResourceText->SetText(FText::GetEmpty());
	//}
	if (RankText)
	{
		RankText->SetText(FText::GetEmpty());
	}
	//if (RewardScrollBox)
	//{
	//	RewardScrollBox->ClearChildren();
	//}
}