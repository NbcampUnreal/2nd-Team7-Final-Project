#include "UI/UIObject/GeneralOptionWidget.h"
#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"

#include "Framework/GameInstance/LCOptionManager.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Engine/GameEngine.h"

#include "LastCanary.h"

void UGeneralOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>();
	if (OptionManager)
	{
		PopulateResolutionOptions();

		if (MasterVolumeSlider)
		{
			MasterVolumeSlider->SetValue(OptionManager->MasterVolume);
			MasterVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnMasterVolumeChanged);
		}
		if (BGMVolumeSlider)
		{
			BGMVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnBGMVolumeChanged);
		}
		if (EffectVolumeSlider)
		{
			EffectVolumeSlider->SetValue(OptionManager->EffectVolume);
			EffectVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnEffectVolumeChanged);
		}
		if (MouseSensitivitySlider)
		{
			MouseSensitivitySlider->SetValue(OptionManager->MouseSensitivity);
			MouseSensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnSensitivityChanged);
		}
		if (BrightnessSlider)
		{
			BrightnessSlider->SetValue(OptionManager->Brightness);
			BrightnessSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnBrightnessChanged);
		}
		if (ResolutionComboBox)
		{
			ResolutionComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnResolutionChanged);
		}
		if (FullScreenCheckBox)
		{
			FullScreenCheckBox->SetIsChecked(OptionManager->ScreenMode == EScreenMode::FullScreen);
			FullScreenCheckBox->OnCheckStateChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnScreenModeChanged);
		}
	}
}

void UGeneralOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (MasterVolumeSlider)
	{
		MasterVolumeSlider->OnValueChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnMasterVolumeChanged);
	}
	if (BGMVolumeSlider)
	{
		BGMVolumeSlider->OnValueChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnBGMVolumeChanged);
	}
	if (EffectVolumeSlider)
	{
		EffectVolumeSlider->OnValueChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnEffectVolumeChanged);
	}
	if (MouseSensitivitySlider)
	{
		MouseSensitivitySlider->OnValueChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnSensitivityChanged);
	}
	if (BrightnessSlider)
	{
		BrightnessSlider->OnValueChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnBrightnessChanged);
	}
	if (ResolutionComboBox)
	{
		ResolutionComboBox->OnSelectionChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnResolutionChanged);
	}
	if (FullScreenCheckBox)
	{
		FullScreenCheckBox->OnCheckStateChanged.RemoveDynamic(this, &UGeneralOptionWidget::OnScreenModeChanged);
	}
}

void UGeneralOptionWidget::OnMasterVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Master Volume Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MasterVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UGeneralOptionWidget::OnBGMVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("BGM Volume Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->BGMVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UGeneralOptionWidget::OnEffectVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Effect Volume Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->EffectVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UGeneralOptionWidget::OnSensitivityChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("SenSitivity Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MouseSensitivity = Value;
	}
}

void UGeneralOptionWidget::OnBrightnessChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Brightness Changed"));

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->Brightness = Value;
	}
}

void UGeneralOptionWidget::OnScreenModeChanged(bool bIsFullscreen)
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

void UGeneralOptionWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	LOG_Frame_WARNING(TEXT("Resolution Changed : %s"), *SelectedItem);

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

void UGeneralOptionWidget::PopulateResolutionOptions()
{
	if (!ResolutionComboBox) return;

	ResolutionComboBox->ClearOptions();

	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);

	const int32 MaxWidth = DisplayMetrics.PrimaryDisplayWidth;
	const int32 MaxHeight = DisplayMetrics.PrimaryDisplayHeight;

	TArray<FIntPoint> CandidateResolutions = {
		{1280, 720}, {1366, 768}, {1600, 900},
		{1920, 1080}, {2560, 1440}, {3840, 2160}
	};

	for (const FIntPoint& Res : CandidateResolutions)
	{
		if (Res.X <= MaxWidth && Res.Y <= MaxHeight)
		{
			const FString ResString = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
			ResolutionComboBox->AddOption(ResString);
		}
	}

	ResolutionComboBox->SetSelectedIndex(0);
}