#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ChecklistQuestionEntryWidget.generated.h"

/**
 * 
 */
class UChecklistWidget;
UCLASS()
class LASTCANARY_API UChecklistQuestionEntryWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    void InitChecklistQuestionEntry(FText InQuestionText, int32 InIndex, UChecklistWidget* InOwnerWidget);

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* QuestionText;
    UPROPERTY(meta = (BindWidget))
    class UCheckBox* AnswerCheckBox;
    UPROPERTY(Transient, meta = (BindWidgetAnim))
    UWidgetAnimation* RevealEntryAnim;

    UFUNCTION()
    void OnAnswerChanged(bool bIsChecked);

private:
    int32 QuestionIndex;
    UChecklistWidget* OwnerWidget;
};