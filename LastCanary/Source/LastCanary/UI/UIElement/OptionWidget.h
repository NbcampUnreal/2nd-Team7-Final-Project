#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "Components/Slider.h"
#include "LastCanary.h"
#include "OptionWidget.generated.h"

class UComboBoxString;
class UCheckBox;
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

	template<typename UserClass>
	void BindSlider(USlider* Slider, float InitValue, UserClass* Obj, void (UserClass::* Func)(float));
	template<typename UserClass>
	void UnbindSlider(USlider* Slider, UserClass* Obj, void (UserClass::* Func)(float));
};

template<typename UserClass>
inline void UOptionWidget::BindSlider(USlider* Slider, float InitValue, UserClass* Obj, void(UserClass::* Func)(float))
{
	if (Slider)
	{
		Slider->SetValue(InitValue);
		Slider->OnValueChanged.AddUniqueDynamic(Obj, Func);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Slider is null"));
	}
}

template<typename UserClass>
inline void UOptionWidget::UnbindSlider(USlider* Slider, UserClass* Obj, void(UserClass::* Func)(float))
{
	if (Slider)
	{
		Slider->OnValueChanged.RemoveDynamic(Obj, Func);
	}
}