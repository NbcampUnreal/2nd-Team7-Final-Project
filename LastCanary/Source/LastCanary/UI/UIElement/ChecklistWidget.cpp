#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIObject/ChecklistQuestionEntryWidget.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Algo/AllOf.h"
#include "LastCanary.h"

void UChecklistWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (SubmitButton)
	{
		SubmitButton->SetVisibility(ESlateVisibility::Hidden);
		SubmitButton->OnClicked.AddUniqueDynamic(this, &UChecklistWidget::SubmitChecklist);
	}
}

void UChecklistWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (SubmitButton)
	{
		SubmitButton->OnClicked.RemoveDynamic(this, &UChecklistWidget::SubmitChecklist);
	}
}

void UChecklistWidget::InitWithQuestions(const TArray<FChecklistQuestion>& InQuestions, AChecklistManager* InManager)
{
	Questions = InQuestions;
	ChecklistManager = InManager;

	if (QuestionScrollBox == nullptr)
	{
		return;
	}
	if (QuestionEntryClass == nullptr)
	{
		return;
	}

	QuestionScrollBox->ClearChildren();
	CurrentRevealIndex = 0;

	GetWorld()->GetTimerManager().SetTimer(
		EntryAddTimerHandle,
		this,
		&UChecklistWidget::AddNextEntry,
		0.3f,
		true
	);
}

void UChecklistWidget::AddNextEntry()
{
	if (!QuestionEntryClass || !QuestionScrollBox || !Questions.IsValidIndex(CurrentRevealIndex))
	{
		GetWorld()->GetTimerManager().ClearTimer(EntryAddTimerHandle);
		return;
	}

	const FChecklistQuestion& Question = Questions[CurrentRevealIndex];
	UChecklistQuestionEntryWidget* Entry = CreateWidget<UChecklistQuestionEntryWidget>(this, QuestionEntryClass);
	if (Entry)
	{
		Entry->InitChecklistQuestionEntry(Question.QuestionText, CurrentRevealIndex, this);
		QuestionScrollBox->AddChild(Entry);
	}
	CurrentRevealIndex++;
}

void UChecklistWidget::UpdateAnswer(int32 QuestionIndex, bool bAnswer)
{
	if (Questions.IsValidIndex(QuestionIndex))
	{
		Questions[QuestionIndex].bAnswer = bAnswer;
		Questions[QuestionIndex].bIsAnswered = true;
		UpdateSubmitButtonVisibility();
	}
}

void UChecklistWidget::UpdateSubmitButtonVisibility()
{
	if (SubmitButton == nullptr)
	{
		return;
	}

	const bool bAllAnswered = Algo::AllOf(Questions, [](const FChecklistQuestion& Q) {
		return Q.bIsAnswered;
		});

	SubmitButton->SetVisibility(bAllAnswered ? ESlateVisibility::Visible : ESlateVisibility::Hidden);

	if (bAllAnswered && FadeInSubmitAnim)
	{
		PlayAnimation(FadeInSubmitAnim);
		LOG_Frame_WARNING(TEXT("ChecklistWidget - SubmitButton 애니메이션 재생"));
	}
}

void UChecklistWidget::SubmitChecklist()
{
	if (!ChecklistManager)
	{
		LOG_Frame_WARNING(TEXT("SubmitChecklist - ChecklistManager is null!"));
		return;
	}

	LOG_Frame_WARNING(TEXT("SubmitChecklist - 호출 OK"));

	if (APlayerController* PC = GetOwningPlayer())
	{
		LOG_Frame_WARNING(TEXT("ChecklistWidget - 서버에 제출 요청"));
		ChecklistManager->Server_SubmitChecklist(PC, Questions); // 결과 계산은 서버에서
	}

	PlayRevealAnimation(); // 제출 후 연출
}

void UChecklistWidget::PlayRevealAnimation()
{
	if (RevealSignatureAnim)
	{
		LOG_Frame_WARNING(TEXT("ChecklistWidget - RevealSignatureAnim 재생 시작"));
		FWidgetAnimationDynamicEvent AnimFinishedDelegate;
		AnimFinishedDelegate.BindDynamic(this, &UChecklistWidget::OnRevealAnimationFinished);
		BindToAnimationFinished(RevealSignatureAnim, AnimFinishedDelegate);
		PlayAnimation(RevealSignatureAnim);
	}
	else
	{
		LOG_Frame_WARNING(TEXT("ChecklistWidget - RevealSignatureAnim 없음 → 즉시 종료"));
		OnRevealAnimationFinished();
	}
}

void UChecklistWidget::OnRevealAnimationFinished()
{
	LOG_Frame_WARNING(TEXT("ChecklistWidget - RevealAnimationFinished 호출 → 위젯 제거"));
	RemoveFromParent();  // 끝
}