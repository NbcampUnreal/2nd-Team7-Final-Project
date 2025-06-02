#include "UI/UIElement/InGameHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInGameHUD::NativeDestruct()
{
	Super::NativeDestruct();
}

void UInGameHUD::UpdateStaminaBar(float StaminaPercent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(StaminaPercent);
	}
}

void UInGameHUD::UpdateLowHealthEffect(float CurrentHP, float MaxHP)
{
	if (!LowHealthEffectImage)
	{
		return;
	}

	const float HealthPercent = CurrentHP / MaxHP;

	// 체력이 낮을수록 더 진하게 (알파값 증가)
	float Alpha = FMath::Clamp(1.0f - HealthPercent, 0.0f, 1.0f);

	LowHealthEffectImage->SetColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, Alpha));
}

void UInGameHUD::SetInteractMessage(const FString& Message)
{
	if (InteractMessageText)
	{
		InteractMessageText->SetText(FText::FromString(Message));
	}
}

void UInGameHUD::SetInteractMessageVisible(bool bVisible)
{
	if (InteractMessageText)
	{
		InteractMessageText->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
	}
}
