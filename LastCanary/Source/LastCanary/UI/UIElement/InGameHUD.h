#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InGameHUD.generated.h"

/**
 * 
 */
class UProgressBar;
class UTextBlock;
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
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractMessageText;

	UFUNCTION()
	void UpdateHealthBar(float HealthPercent);
	UFUNCTION()
	void UpdateStaminaBar(float StaminaPercent);

public:
	UFUNCTION(BlueprintCallable)
	void SetInteractMessage(const FString& Message);

	UFUNCTION(BlueprintCallable)
	void SetInteractMessageVisible(bool bVisible);
};
