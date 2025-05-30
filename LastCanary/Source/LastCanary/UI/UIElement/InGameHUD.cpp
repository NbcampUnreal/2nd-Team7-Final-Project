#include "UI/UIElement/InGameHUD.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

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
