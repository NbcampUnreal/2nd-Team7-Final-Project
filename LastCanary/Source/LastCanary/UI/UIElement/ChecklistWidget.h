#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/ChecklistQuestion.h"
#include "ChecklistWidget.generated.h"

/**
 * 
 */
class AChecklistManager;
class UScrollBox;
class UChecklistQuestionEntryWidget;
class UButton;

UCLASS()
class LASTCANARY_API UChecklistWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UFUNCTION(BlueprintCallable)
    void InitWithQuestions(const TArray<FChecklistQuestion>& InQuestions, AChecklistManager* InManager);
    UFUNCTION(BlueprintCallable)
    void SubmitChecklist();
    UFUNCTION(BlueprintCallable)
    void UpdateAnswer(int32 QuestionIndex, bool bAnswer);

protected:
    UPROPERTY(meta = (BindWidget))
    UScrollBox* QuestionScrollBox;
    UPROPERTY()
    TArray<FChecklistQuestion> Questions;
    
    UPROPERTY()
    AChecklistManager* ChecklistManager;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checklist")
    TSubclassOf<UChecklistQuestionEntryWidget> QuestionEntryClass;
};
