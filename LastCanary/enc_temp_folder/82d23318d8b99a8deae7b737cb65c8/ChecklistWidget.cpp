#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIObject/ChecklistQuestionEntryWidget.h"
#include "Framework/Manager/ChecklistManager.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"

#include "LastCanary.h"

void UChecklistWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (SubmitButton)
    {
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

void UChecklistWidget::OnRevealAnimationFinished()
{
    GetWorld()->GetTimerManager().SetTimer(
        SubmitDelayTimerHandle,
        this,
        &UChecklistWidget::FinalizeSubmitChecklist,
        0.5f,
        false
    );
}

void UChecklistWidget::FinalizeSubmitChecklist()
{
    if (ChecklistManager)
    {
        ChecklistManager->OnChecklistSubmitted(Questions);
    }

    RemoveFromParent();
}

void UChecklistWidget::InitWithQuestions(const TArray<FChecklistQuestion>& InQuestions, AChecklistManager* InChecklistManager)
{
    Questions = InQuestions;
    ChecklistManager = InChecklistManager;

    if (!QuestionScrollBox || !QuestionEntryClass)
    {
        return;
    }

    QuestionScrollBox->ClearChildren();

    for (int32 i = 0; i < Questions.Num(); ++i)
    {
        UChecklistQuestionEntryWidget* Entry = CreateWidget<UChecklistQuestionEntryWidget>(this, QuestionEntryClass);
        if (Entry)
        {
            Entry->InitChecklistQuestionEntry(Questions[i].QuestionText, i, this);
            QuestionScrollBox->AddChild(Entry);
            LOG_Frame_WARNING(TEXT("Entry Added!"));
        }
    }
}

void UChecklistWidget::UpdateAnswer(int32 QuestionIndex, bool bAnswer)
{
    if (Questions.IsValidIndex(QuestionIndex))
    {
        Questions[QuestionIndex].bAnswer = bAnswer;
        Questions[QuestionIndex].bIsAnswered = true;

        LOG_Frame_WARNING(TEXT("Checklist Entry Updated! Index: %d, Answer: %s"),
            QuestionIndex,
            bAnswer ? TEXT("True") : TEXT("False"));
    }
}

void UChecklistWidget::SubmitChecklist()
{
    if (RevealSignatureAnim)
    {
        PlayAnimation(RevealSignatureAnim);

        FWidgetAnimationDynamicEvent AnimFinishedDelegate;
        AnimFinishedDelegate.BindDynamic(this, &UChecklistWidget::OnRevealAnimationFinished);
        BindToAnimationFinished(RevealSignatureAnim, AnimFinishedDelegate);
    }
    else
    {
        OnRevealAnimationFinished(); // 애니메이션이 없을 경우 바로 호출
    }
}

