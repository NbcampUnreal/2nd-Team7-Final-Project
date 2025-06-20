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
class UInputAction;
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
	UPROPERTY(meta = (BindWidget))
	UTextBlock* VoiceKeyGuideText;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	UWidgetAnimation* TakeDamageAnim;

public:
	UPROPERTY(EditAnywhere, Category = "UI")
	UInputAction* IA_Voice;

	UFUNCTION(BlueprintCallable)
	void PlayTakeDamageAnim();
	UFUNCTION(BlueprintCallable)
	void UpdateHPBar(float HealthPercent);
	UFUNCTION(BlueprintCallable)
	void UpdateStaminaBar(float StaminaPercent);
	UFUNCTION(BlueprintCallable)
	void SetInteractMessage(const FString& Message);
	UFUNCTION(BlueprintCallable)
	void SetInteractMessageVisible(bool bVisible);
	UFUNCTION(BlueprintCallable)
	void SetVoiceKeyGuideText();
	UFUNCTION(BlueprintCallable)
	FString GetCurrentKeyNameForAction(UInputAction* InputAction) const;
};
