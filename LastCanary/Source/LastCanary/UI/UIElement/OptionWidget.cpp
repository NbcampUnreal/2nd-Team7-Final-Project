#include "UI/UIElement/OptionWidget.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Widgets/Input/SComboBox.h"

#include "Framework/GameInstance/LCOptionManager.h"

#include "LastCanary.h"

void UOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>();
	if (OptionManager)
	{
		BindSlider(MasterVolumeSlider, OptionManager->MasterVolume, this, &UOptionWidget::OnMasterVolumeChanged);
		BindSlider(BGMVolumeSlider, OptionManager->BGMVolume, this, &UOptionWidget::OnBGMVolumeChanged);
		BindSlider(EffectVolumeSlider, OptionManager->EffectVolume, this, &UOptionWidget::OnEffectVolumeChanged);
		BindSlider(MouseSensitivitySlider, OptionManager->MouseSensitivity, this, &UOptionWidget::OnSensitivityChanged);
		BindSlider(BrightnessSlider, OptionManager->Brightness, this, &UOptionWidget::OnBrightnessChanged);

		if (ResolutionComboBox)
		{
			ResolutionComboBox->ClearOptions();
			ResolutionComboBox->AddOption(TEXT("1280x720"));
			ResolutionComboBox->AddOption(TEXT("1600x900"));
			ResolutionComboBox->AddOption(TEXT("1920x1080"));
			ResolutionComboBox->SetSelectedIndex(OptionManager->ResolutionIndex);
		}

		if (FullScreenCheckBox)
		{
			FullScreenCheckBox->SetIsChecked(OptionManager->ScreenMode == EScreenMode::FullScreen);
			FullScreenCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UOptionWidget::OnScreenModeChanged);
		}
	}
}
 
void UOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	UnbindSlider(MasterVolumeSlider, this, &UOptionWidget::OnMasterVolumeChanged);
	UnbindSlider(BGMVolumeSlider, this, &UOptionWidget::OnBGMVolumeChanged);
	UnbindSlider(EffectVolumeSlider, this, &UOptionWidget::OnEffectVolumeChanged);
	UnbindSlider(MouseSensitivitySlider, this, &UOptionWidget::OnSensitivityChanged);
	UnbindSlider(BrightnessSlider, this, &UOptionWidget::OnBrightnessChanged);
	if (FullScreenCheckBox)
	{
		FullScreenCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UOptionWidget::OnScreenModeChanged);
	}
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.RemoveDynamic(this, &UOptionWidget::OnResolutionChanged);
	}
}

void UOptionWidget::OnMasterVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Master Volume Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MasterVolume = Value;
	}
}

void UOptionWidget::OnBGMVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("BGM Volume Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->BGMVolume = Value;
	}
}

void UOptionWidget::OnEffectVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Effect Volume Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->EffectVolume = Value;
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

void UOptionWidget::OnScreenModeChanged(bool bIsFullscreen)
{
	LOG_Frame_WARNING(TEXT("Screen Mode Changed : %s"), bIsFullscreen ? TEXT("FullScreen") : TEXT("Windowed"));
	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		if (bIsFullscreen)
		{
			OptionManager->ChangeScreen(EScreenMode::FullScreen);
		}
		else
		{
			OptionManager->ChangeScreen(EScreenMode::Window);
		}
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

