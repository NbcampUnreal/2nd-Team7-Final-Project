#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "Components/Slider.h"
#include "LastCanary.h"
#include "GeneralOptionWidget.generated.h"

/**
 * 
 */
class UComboBoxString;
class UCheckBox;
UCLASS()
class LASTCANARY_API UGeneralOptionWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	USlider* MasterVolumeSlider;
	UPROPERTY(meta = (BindWidget))
	USlider* BGMVolumeSlider;
	UPROPERTY(meta = (BindWidget))
	USlider* EffectVolumeSlider;
	UPROPERTY(meta = (BindWidget))
	USlider* MouseSensitivitySlider;
	UPROPERTY(meta = (BindWidget))
	USlider* BrightnessSlider;
	UPROPERTY(meta = (BindWidget))
	UCheckBox* FullScreenCheckBox;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ResolutionComboBox;

public:
	UFUNCTION()
	void OnMasterVolumeChanged(float Value);
	UFUNCTION()
	void OnBGMVolumeChanged(float Value);
	UFUNCTION()
	void OnEffectVolumeChanged(float Value);
	UFUNCTION()
	void OnSensitivityChanged(float Value);
	UFUNCTION()
	void OnBrightnessChanged(float Value);
	UFUNCTION()
	void OnScreenModeChanged(bool bIsFullscreen);
	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	void PopulateResolutionOptions();
};
