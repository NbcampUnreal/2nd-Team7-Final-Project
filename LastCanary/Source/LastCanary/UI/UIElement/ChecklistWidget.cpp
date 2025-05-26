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

void UChecklistWidget::UpdateSubmitButtonVisibility()
{
    if (SubmitButton == nullptr)
    {
        return;
    }

    const bool bAllAnswered = std::all_of(
        Questions.begin(), Questions.end(),
        [](const FChecklistQuestion& Q)
        {
            return Q.bIsAnswered;
        });

    if (bAllAnswered)
    {
        SubmitButton->SetVisibility(ESlateVisibility::Visible);

        if (FadeInSubmitAnim)
        {
            PlayAnimation(FadeInSubmitAnim);
            LOG_Frame_WARNING(TEXT("ChecklistWidget - SubmitButton 애니메이션 재생"));
        }
    }
    else
    {
        SubmitButton->SetVisibility(ESlateVisibility::Hidden);
    }
}

void UChecklistWidget::InitWithQuestions(const TArray<FChecklistQuestion>& InQuestions, AChecklistManager* InChecklistManager)
{
    Questions = InQuestions;
    ChecklistManager = InChecklistManager;

    if (QuestionScrollBox == nullptr)
    {
        return;
    }
    if (QuestionEntryClass == nullptr)
    {
        return;
    }

    QuestionScrollBox->ClearChildren();

    QueuedQuestions = Questions;
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
    if (!QuestionEntryClass || !QuestionScrollBox || !QueuedQuestions.IsValidIndex(CurrentRevealIndex))
    {
        GetWorld()->GetTimerManager().ClearTimer(EntryAddTimerHandle);
        return;
    }

    const FChecklistQuestion& Question = QueuedQuestions[CurrentRevealIndex];

    UChecklistQuestionEntryWidget* Entry = CreateWidget<UChecklistQuestionEntryWidget>(this, QuestionEntryClass);
    if (Entry)
    {
        Entry->InitChecklistQuestionEntry(Question.QuestionText, CurrentRevealIndex, this);
        QuestionScrollBox->AddChild(Entry);

        LOG_Frame_WARNING(TEXT("Entry 순차 추가: %s"), *Question.QuestionText.ToString());
    }

    CurrentRevealIndex++;
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

        UpdateSubmitButtonVisibility();
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
        OnRevealAnimationFinished(); 
    }
}

