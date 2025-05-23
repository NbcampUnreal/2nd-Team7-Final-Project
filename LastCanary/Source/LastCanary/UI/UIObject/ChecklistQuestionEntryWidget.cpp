#include "UI/UIObject/ChecklistQuestionEntryWidget.h"
#include "Components/TextBlock.h"
#include "Components/CheckBox.h"
#include "UI/UIElement/ChecklistWidget.h"

void UChecklistQuestionEntryWidget::InitChecklistQuestionEntry(FText InQuestionText, int32 InIndex, UChecklistWidget* InOwnerWidget)
{
    QuestionIndex = InIndex;
    OwnerWidget = InOwnerWidget;

    if (QuestionText)
    {
        QuestionText->SetText(InQuestionText);
    }

    if (AnswerCheckBox)
    {
        AnswerCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UChecklistQuestionEntryWidget::OnAnswerChanged);
    }
}

void UChecklistQuestionEntryWidget::OnAnswerChanged(bool bIsChecked)
{
    if (OwnerWidget)
    {
        OwnerWidget->UpdateAnswer(QuestionIndex, bIsChecked);
    }
}
