#include "UI/UIElement/InGameHUD.h"
#include "Components/ProgressBar.h"

void UInGameHUD::NativeConstruct()
{
	Super::NativeConstruct();
}

void UInGameHUD::NativeDestruct()
{
	Super::NativeDestruct();
}

void UInGameHUD::UpdateHealthBar(float HealthPercent)
{
	if (HealthBar)
	{
		HealthBar->SetPercent(HealthPercent);
	}
}

void UInGameHUD::UpdateStaminaBar(float StaminaPercent)
{
	if (StaminaBar)
	{
		StaminaBar->SetPercent(StaminaPercent);
	}
}