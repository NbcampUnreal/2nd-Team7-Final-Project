#include "UI/UIElement/OptionWidget.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Widgets/Input/SComboBox.h"
#include "LastCanary.h"
#include "UI/Manager/LCOptionManager.h"

void UOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>();
	if (OptionManager)
	{
		if (MasterVolumeSlider)
		{
			MasterVolumeSlider->SetValue(OptionManager->MasterVolume);
		}
		if (MouseSensitivitySlider)
		{
			MouseSensitivitySlider->SetValue(OptionManager->MouseSensitivity);
		}
		if (BrightnessSlider)
		{
			BrightnessSlider->SetValue(OptionManager->Brightness);
		}
		if (ResolutionComboBox)
		{
			ResolutionComboBox->ClearOptions();
			ResolutionComboBox->AddOption(TEXT("1280x720"));
			ResolutionComboBox->AddOption(TEXT("1600x900"));
			ResolutionComboBox->AddOption(TEXT("1920x1080"));
			ResolutionComboBox->SetSelectedIndex(OptionManager->ResolutionIndex);
		}
	}

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

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MasterVolume = Value;
	}
}

void UOptionWidget::OnSensitivityChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("SenSitivity Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MouseSensitivity = Value;
	}
}

void UOptionWidget::OnBrightnessChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Brightness Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->Brightness = Value;
	}
}

void UOptionWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	LOG_Frame_WARNING(TEXT("Resolution Changed : %s"), *SelectedItem);

	if (SelectionType != ESelectInfo::Direct && SelectionType != ESelectInfo::OnKeyPress && SelectionType != ESelectInfo::OnMouseClick)
	{
		return;
	}

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->ResolutionIndex = ResolutionComboBox->FindOptionIndex(SelectedItem);

		if (SelectedItem == "1280x720")
		{
			OptionManager->ScreenResolution = FIntPoint(1280, 720);
		}
		else if (SelectedItem == "1600x900")
		{
			OptionManager->ScreenResolution = FIntPoint(1600, 900);
		}
		else if (SelectedItem == "1920x1080")
		{
			OptionManager->ScreenResolution = FIntPoint(1920, 1080);
		}
	}
}

