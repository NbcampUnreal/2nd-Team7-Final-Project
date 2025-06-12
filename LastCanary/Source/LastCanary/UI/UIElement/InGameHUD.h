#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InGameHUD.generated.h"

/**
 * 
 */
class UProgressBar;
class UTextBlock;
class UImage;
UCLASS()
class LASTCANARY_API UInGameHUD : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UImage* LowHealthEffectImage;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InteractMessageText;

public:
	UFUNCTION(BlueprintCallable)
	void UpdateLowHealthEffect(float CurrentHealth, float MaxHealth);
	UFUNCTION(BlueprintCallable)
	void UpdateStaminaBar(float StaminaPercent);
	UFUNCTION(BlueprintCallable)
	void SetInteractMessage(const FString& Message);
	UFUNCTION(BlueprintCallable)
	void SetInteractMessageVisible(bool bVisible);
};
