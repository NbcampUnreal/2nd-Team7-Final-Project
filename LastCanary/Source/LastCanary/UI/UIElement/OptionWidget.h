#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "OptionWidget.generated.h"

class USlider;
class UComboBoxString;

UCLASS()
class LASTCANARY_API UOptionWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	USlider* MasterVolumeSlider;
	UPROPERTY(meta = (BindWidget))
	USlider* MouseSensitivitySlider;
	UPROPERTY(meta = (BindWidget))
	USlider* BrightnessSlider;
	UPROPERTY(meta = (BindWidget))
	UComboBoxString* ResolutionComboBox;

	UFUNCTION()
	void OnVolumeChanged(float Value);
	UFUNCTION()
	void OnSensitivityChanged(float Value);
	UFUNCTION()
	void OnBrightnessChanged(float Value);
	UFUNCTION()
	void OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};