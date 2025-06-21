// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ItemDropQuantityWidget.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnQuantityConfirmed, int32, Quantity);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnQuantityCanceled);

class UTextBlock;
class UEditableTextBox;
class UButton;
class USlider;

UCLASS()
class LASTCANARY_API UItemDropQuantityWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    virtual void NativeConstruct() override;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* ItemNameText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MaxQuantityText;

    UPROPERTY(meta = (BindWidget))
    UEditableTextBox* QuantityInputBox;

    UPROPERTY(meta = (BindWidget))
    USlider* QuantitySlider;

    UPROPERTY(meta = (BindWidget))
    UButton* ConfirmButton;

    UPROPERTY(meta = (BindWidget))
    UButton* CancelButton;

    UPROPERTY(meta = (BindWidget))
    UButton* MaxButton;

    // 델리게이트
    UPROPERTY(BlueprintAssignable)
    FOnQuantityConfirmed OnQuantityConfirmed;

    UPROPERTY(BlueprintAssignable)
    FOnQuantityCanceled OnQuantityCanceled;

    // 위젯 설정
    UFUNCTION(BlueprintCallable)
    void SetupWidget(const FText& ItemName, int32 MaxQuantity);

    // 현재 선택된 개수 가져오기
    UFUNCTION(BlueprintPure)
    int32 GetSelectedQuantity() const;

private:
    int32 MaxQuantity = 1;
    int32 CurrentQuantity = 1;

    // 버튼 이벤트
    UFUNCTION()
    void OnConfirmClicked();

    UFUNCTION()
    void OnCancelClicked();

    UFUNCTION()
    void OnMaxClicked();

    // 슬라이더/텍스트박스 이벤트
    UFUNCTION()
    void OnSliderValueChanged(float Value);

    UFUNCTION()
    void OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod);

    // UI 업데이트
    void UpdateQuantityDisplay();
    void ValidateQuantity();
};
