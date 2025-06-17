#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "GraphicsOption.generated.h"

/**
 * 
 */

class UButton;
class UTextBlock;
class UUniformGridPanel;
class UBorder;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnGraphicsOptionChanged, FName, OptionKey, int32, SelectedIndex);

UCLASS()
class LASTCANARY_API UGraphicsOption : public ULCUserWidgetBase
{
    GENERATED_BODY()

public:
    virtual void NativeConstruct() override;
    virtual void NativeDestruct() override;

    UPROPERTY(BlueprintAssignable)
    FOnGraphicsOptionChanged OnOptionChanged;

    void InitializeOption(FName InOptionKey, const TArray<FText>& InOptions, int32 DefaultIndex = 0, const FText& LabelText = FText::GetEmpty());
    int32 GetCurrentIndex() const { return CurrentIndex; }
    FText GetCurrentOption() const;

protected:
    UPROPERTY(meta = (BindWidget)) 
    UButton* LeftButton;
    UPROPERTY(meta = (BindWidget)) 
    UButton* RightButton;
    UPROPERTY(meta = (BindWidget)) 
    UTextBlock* CurrentOptionText;
    UPROPERTY(meta = (BindWidget)) 
    UTextBlock* TextBlock_Label;
    UPROPERTY(meta = (BindWidget)) 
    UUniformGridPanel* StepIndicators;

    UFUNCTION()
    void OnLeftButtonClicked();

    UFUNCTION()
    void OnRightButtonClicked();

private:
    void UpdateUI();

    FName OptionKey;
    TArray<FText> Options;
    int32 CurrentIndex = 0;
};
