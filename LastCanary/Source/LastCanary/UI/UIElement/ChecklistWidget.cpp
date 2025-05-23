#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIObject/ChecklistQuestionEntryWidget.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Components/ScrollBox.h"

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
        }
    }
}

void UChecklistWidget::UpdateAnswer(int32 QuestionIndex, bool bAnswer)
{
    if (Questions.IsValidIndex(QuestionIndex))
    {
        Questions[QuestionIndex].bAnswer = bAnswer;
        Questions[QuestionIndex].bIsAnswered = true;
    }
}

void UChecklistWidget::SubmitChecklist()
{
    if (ChecklistManager)
    {
        ChecklistManager->OnChecklistSubmitted(Questions);
    }

    RemoveFromParent(); 
}
