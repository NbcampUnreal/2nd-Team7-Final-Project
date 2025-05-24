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

	if (ChecklistDataTable == nullptr)
	{
		LOG_Frame_WARNING(TEXT("ChecklistDataTable is null!"));
		return;
	}

	static const FString ContextString(TEXT("Checklist Load"));
	TArray<FChecklistQuestionRow*> Rows;
	ChecklistDataTable->GetAllRows(ContextString, Rows);

	for (auto* Row : Rows)
	{
		if (Row == nullptr)
		{
			continue;
		}

		FChecklistQuestion NewQuestion;
		NewQuestion.QuestionText = Row->QuestionText;
		NewQuestion.bAnswer = false;
		NewQuestion.bIsAnswered = false;

		Questions.Add(NewQuestion);
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
	if (Evaluator == nullptr)
	{
		Evaluator = NewObject<UResultEvaluator>(this);
		Evaluator->ResourceItemTable = ResourceItemTable;
		Evaluator->ResourceCategoryTable = ResourceCategoryTable;
		Evaluator->RankThresholdTable = RankThresholdTable;
	}

	int32 Surviving = 3; // TODO: GameState 또는 인게임에서 실제 생존자 수 가져오기

	// 예시용 자원 수집 맵 ( TODO : 나중에 인벤토리에서 실제 수집한 자원으로 대체)
	TMap<FName, int32> CollectedResources;
	CollectedResources.Add("Iron", 5);
	CollectedResources.Add("Adamantium", 2);
	CollectedResources.Add("Ruby", 1);

	FGameResultData ResultData = Evaluator->EvaluateResult(
		PlayerAnswers, 
		CorrectAnswers, 
		Surviving, 
		CollectedResources);

	if (ResultMenuClass)
	{
		UResultMenu* ResultWidget = CreateWidget<UResultMenu>(GetWorld(), ResultMenuClass);
		if (ResultWidget)
		{
			// 예시 보상 데이터 (실제로는 Rank 기반으로 보상 계산 가능)
			TArray<FResultRewardEntry> RewardList;
			// TODO : 데이터테이블을 통해 질문마다 점수 다르게 확장
			RewardList.Add({ FText::FromString("Checklist"),
				FText::Format(FText::FromString("Report Accuracy : {0} / {1}"),
				FText::AsNumber(ResultData.CorrectChecklistCount),
				FText::AsNumber(ResultData.TotalChecklistCount)),
				ResultData.CorrectChecklistCount * 100 });
			// TODO : 서버에서 생존자수 검사
			RewardList.Add({ FText::FromString("Survivors"),
				FText::Format(FText::FromString("Survivors Alived : {0}"),
				FText::AsNumber(ResultData.SurvivingPlayerCount)),
				ResultData.SurvivingPlayerCount * 50 });
			// TODO : 인벤토리를 검사해서 점수 계산 
			RewardList.Add({ FText::FromString("Resources"),
				FText::Format(FText::FromString("Resources Score : {0}"),
				FText::AsNumber(ResultData.CollectedResourcePoints)),
				ResultData.CollectedResourcePoints });
			// TODO : 랭크 기준 변경 
			/*RewardList.Add({ FText::FromString("Rank"),
				FText::Format(FText::FromString("Rank : {0}"),
				FText::FromString(ResultData.Rank)),
				0 });*/
			// TODO : UIManager로 중앙집중화

			ResultWidget->AddToViewport();
			ResultWidget->SetRewardEntries(RewardList);
			ResultWidget->SetResourceScoreDetails(ResultData.ResourceScoreDetails);
			ResultWidget->SetTotalGold(ResultData.FinalScore);
			ResultWidget->SetRankText(ResultData.Rank); 
		}
	}
}
