// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIObject/ItemDropQuantityWidget.h"
#include "Components/TextBlock.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
#include "Components/Slider.h"

void UItemDropQuantityWidget::NativeConstruct()
{
    Super::NativeConstruct();

    // 버튼 이벤트 바인딩
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddDynamic(this, &UItemDropQuantityWidget::OnConfirmClicked);
    }

    if (CancelButton)
    {
        CancelButton->OnClicked.AddDynamic(this, &UItemDropQuantityWidget::OnCancelClicked);
    }

    if (MaxButton)
    {
        MaxButton->OnClicked.AddDynamic(this, &UItemDropQuantityWidget::OnMaxClicked);
    }

    // 슬라이더 이벤트 바인딩
    if (QuantitySlider)
    {
        QuantitySlider->OnValueChanged.AddDynamic(this, &UItemDropQuantityWidget::OnSliderValueChanged);
    }

    // 텍스트박스 이벤트 바인딩
    if (QuantityInputBox)
    {
        QuantityInputBox->OnTextCommitted.AddDynamic(this, &UItemDropQuantityWidget::OnTextCommitted);
    }
}

void UItemDropQuantityWidget::SetupWidget(const FText& ItemName, int32 InMaxQuantity)
{
    MaxQuantity = FMath::Max(1, InMaxQuantity);
    CurrentQuantity = 1;

    if (ItemNameText)
    {
        ItemNameText->SetText(FText::Format(NSLOCTEXT("UI", "DropItemFormat", "Drop {0}"), ItemName));
    }

    if (MaxQuantityText)
    {
        MaxQuantityText->SetText(FText::Format(NSLOCTEXT("UI", "MaxQuantityFormat", "Max: {0}"), FText::AsNumber(MaxQuantity)));
    }

    if (QuantitySlider)
    {
        QuantitySlider->SetMinValue(1.0f);
        QuantitySlider->SetMaxValue(static_cast<float>(MaxQuantity));
        QuantitySlider->SetValue(1.0f);
    }

    UpdateQuantityDisplay();
}

int32 UItemDropQuantityWidget::GetSelectedQuantity() const
{
    return CurrentQuantity;
}

void UItemDropQuantityWidget::OnConfirmClicked()
{
    ValidateQuantity();
    OnQuantityConfirmed.Broadcast(CurrentQuantity);
    RemoveFromParent();
}

void UItemDropQuantityWidget::OnCancelClicked()
{
    OnQuantityCanceled.Broadcast();
    RemoveFromParent();
}

void UItemDropQuantityWidget::OnMaxClicked()
{
    CurrentQuantity = MaxQuantity;
    if (QuantitySlider)
    {
        QuantitySlider->SetValue(static_cast<float>(MaxQuantity));
    }
    UpdateQuantityDisplay();
}

void UItemDropQuantityWidget::OnSliderValueChanged(float Value)
{
    CurrentQuantity = FMath::RoundToInt(Value);
    UpdateQuantityDisplay();
}

void UItemDropQuantityWidget::OnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
    int32 NewQuantity = FCString::Atoi(*Text.ToString());
    CurrentQuantity = FMath::Clamp(NewQuantity, 1, MaxQuantity);
    
    if (QuantitySlider)
    {
        QuantitySlider->SetValue(static_cast<float>(CurrentQuantity));
    }
    
    UpdateQuantityDisplay();
}

void UItemDropQuantityWidget::UpdateQuantityDisplay()
{
    if (QuantityInputBox)
    {
        QuantityInputBox->SetText(FText::AsNumber(CurrentQuantity));
    }   
}

void UItemDropQuantityWidget::ValidateQuantity()
{
    CurrentQuantity = FMath::Clamp(CurrentQuantity, 1, MaxQuantity);
}
