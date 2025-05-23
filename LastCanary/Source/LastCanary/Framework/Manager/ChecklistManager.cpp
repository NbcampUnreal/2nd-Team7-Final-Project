#include "Framework/Manager/ChecklistManager.h"
#include "Framework/Manager/ResultEvaluator.h"
#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIElement/ResultMenu.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DataTable.h"
#include "DataType/GameResultData.h"

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
		if (!Row) continue;

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
			Widget->InitWithQuestions(Questions, this); // 위젯에 질문 리스트 + Submit 콜백 전달
		}
	}
}

void AChecklistManager::OnChecklistSubmitted(const TArray<FChecklistQuestion>& PlayerAnswers)
{
	if (!Evaluator)
	{
		Evaluator = NewObject<UResultEvaluator>(this);
	}

	int32 Surviving = 3; // 실제 생존자 수 로직으로 대체 예정
	int32 Resource = 120; // 실제 자원 수치로 대체 예정

	// 평가
	FGameResultData ResultData = Evaluator->EvaluateResult(PlayerAnswers, CorrectAnswers, Surviving, Resource);

	// 결과 UI 띄우기
	if (ResultMenuClass)
	{
		UResultMenu* ResultWidget = CreateWidget<UResultMenu>(GetWorld(), ResultMenuClass);
		if (ResultWidget)
		{
			// 예시 보상 데이터 (실제로는 Rank 기반으로 보상 계산 가능)
			TArray<FResultRewardEntry> RewardList;
			RewardList.Add({ FText::FromString(TEXT("Gold")), FText::FromString(TEXT("Rank Bonus")), ResultData.FinalScore });

			ResultWidget->AddToViewport();
			ResultWidget->SetRewardEntries(RewardList);
			ResultWidget->SetTotalGold(ResultData.FinalScore);
		}
	}
}
