#include "Framework/Manager/ChecklistManager.h"
#include "Framework/Manager/ResultEvaluator.h"

#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIElement/ResultMenu.h"

#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "DataType/GameResultData.h"

#include "LastCanary.h"

AChecklistManager::AChecklistManager()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = false;
}

void AChecklistManager::BeginPlay()
{
	Super::BeginPlay();
	InitializeQuestionsFromDataTable();
}

void AChecklistManager::InitializeQuestionsFromDataTable()
{
	Questions.Empty();
	CorrectAnswers.Empty();

	if (!ChecklistDataTable)
	{
		UE_LOG(LogTemp, Warning, TEXT("ChecklistDataTable is null!"));
		return;
	}

	static const FString ContextString(TEXT("Checklist Load"));
	TArray<FChecklistQuestionRow*> Rows;
	ChecklistDataTable->GetAllRows(ContextString, Rows);

	for (auto* Row : Rows)
	{
		if (!Row)
		{
			continue;
		}

		FChecklistQuestion Q;
		Q.QuestionText = Row->QuestionText;
		Q.bAnswer = false;
		Q.bIsAnswered = false;

		Questions.Add(Q);
		CorrectAnswers.Add(Row->bCorrectAnswer);
	}
}

void AChecklistManager::StartChecklist()
{
	if (ChecklistWidgetClass)
	{
		UChecklistWidget* Widget = CreateWidget<UChecklistWidget>(GetWorld(), ChecklistWidgetClass);
		if (Widget)
		{
			Widget->AddToViewport();
			Widget->InitWithQuestions(Questions, this); 
		}
	}
}

void AChecklistManager::OnChecklistSubmitted(const TArray<FChecklistQuestion>& PlayerAnswers)
{
	if (!Evaluator)
	{
		Evaluator = NewObject<UResultEvaluator>(this);
	}

	int32 Surviving = 3; 
	int32 Resource = 120;

	FGameResultData ResultData = Evaluator->EvaluateResult(PlayerAnswers, CorrectAnswers, Surviving, Resource);

	if (ResultMenuClass)
	{
		UResultMenu* ResultWidget = CreateWidget<UResultMenu>(GetWorld(), ResultMenuClass);
		if (ResultWidget)
		{
			// 예시 보상 데이터 (실제로는 Rank 기반으로 보상 계산 가능)
			TArray<FResultRewardEntry> RewardList;

			RewardList.Add({ FText::FromString("Checklist"),
				FText::Format(FText::FromString("Report Accuracy : {0}/{1}"),
				FText::AsNumber(ResultData.CorrectChecklistCount),
				FText::AsNumber(ResultData.TotalChecklistCount)),
				ResultData.CorrectChecklistCount * 100 });

			RewardList.Add({ FText::FromString("Survivors"),
				FText::Format(FText::FromString("Survivors Alived : {0}"),
				FText::AsNumber(ResultData.SurvivingPlayerCount)),
				ResultData.SurvivingPlayerCount * 50 });

			RewardList.Add({ FText::FromString("Resources"),
				FText::Format(FText::FromString("Resources Score : {0}"),
				FText::AsNumber(ResultData.CollectedResourcePoints)),
				ResultData.CollectedResourcePoints });

			RewardList.Add({ FText::FromString("Rank"),
				FText::Format(FText::FromString("Rank : {0}"),
				FText::FromString(ResultData.Rank)),
				0 });

			ResultWidget->AddToViewport();
			ResultWidget->SetRewardEntries(RewardList);
			ResultWidget->SetTotalGold(ResultData.FinalScore);
		}
	}
}
