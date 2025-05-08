#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InGameHUD.generated.h"

/**
 * 
 */
class UProgressBar;
UCLASS()
class LASTCANARY_API UInGameHUD : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;

	UFUNCTION()
	void UpdateHealthBar(float HealthPercent);
	UFUNCTION()
	void UpdateStaminaBar(float StaminaPercent);
};
