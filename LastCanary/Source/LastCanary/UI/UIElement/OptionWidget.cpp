#include "UI/UIElement/OptionWidget.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Widgets/Input/SComboBox.h"
#include "LastCanary.h"

void UOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UOptionWidget::OnVolumeChanged);
	}
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UOptionWidget::OnSensitivityChanged);
	}
	if (BrightnessSlider)
	{
		BrightnessSlider->OnValueChanged.AddUniqueDynamic(this, &UOptionWidget::OnBrightnessChanged);
	}
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UOptionWidget::OnResolutionChanged);
	}
}

void UOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UOptionWidget::OnVolumeChanged);
	}
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UOptionWidget::OnSensitivityChanged);
	}
	if (BrightnessSlider)
	{
		BrightnessSlider->OnValueChanged.RemoveDynamic(this, &UOptionWidget::OnBrightnessChanged);
	}
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.RemoveDynamic(this, &UOptionWidget::OnResolutionChanged);
	}
}

void UOptionWidget::OnVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Volume Changed"));
}

void UOptionWidget::OnSensitivityChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("SenSitivity Changed"));
}

void UOptionWidget::OnBrightnessChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Brightness Changed"));
}

void UOptionWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	LOG_Frame_WARNING(TEXT("Resolution Changed"));
}

