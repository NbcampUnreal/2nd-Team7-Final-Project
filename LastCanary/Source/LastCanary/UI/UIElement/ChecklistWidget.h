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

protected:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(meta = (BindWidget))
    UButton* SubmitButton;
    UPROPERTY(meta = (BindWidget))
    UScrollBox* QuestionScrollBox;
    UPROPERTY(meta = (BindWidgetAnim), Transient)
    UWidgetAnimation* RevealSignatureAnim;
    UPROPERTY(Transient, meta = (BindWidgetAnim), BlueprintReadOnly)
    UWidgetAnimation* FadeInSubmitAnim;

    FTimerHandle SubmitDelayTimerHandle;
    FTimerHandle EntryAddTimerHandle;

    UPROPERTY()
    TArray<FChecklistQuestion> Questions;
    UPROPERTY()
    AChecklistManager* ChecklistManager;
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Checklist")
    TSubclassOf<UChecklistQuestionEntryWidget> QuestionEntryClass;

    UFUNCTION()
    void OnRevealAnimationFinished();

    void UpdateSubmitButtonVisibility();
    void AddNextEntry();

public:
    void InitWithQuestions(const TArray<FChecklistQuestion>& InQuestions, AChecklistManager* InManager);

    UFUNCTION()
    void SubmitChecklist();

    void PlayRevealAnimation();

    UFUNCTION(BlueprintCallable)
    void UpdateAnswer(int32 QuestionIndex, bool bAnswer);
    
private:
    int32 CurrentRevealIndex = 0;
};
