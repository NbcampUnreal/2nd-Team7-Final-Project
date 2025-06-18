#include "UI/UIObject/GeneralOptionWidget.h"

#include "Components/Slider.h"
#include "Components/ComboBoxString.h"
#include "Components/CheckBox.h"
#include "Components/TextBlock.h"

#include "Framework/GameInstance/LCOptionManager.h"
#include "Framework/Application/SlateApplication.h"
#include "GenericPlatform/GenericPlatformMisc.h"
#include "Engine/GameEngine.h"

#include "SaveGame/LCLocalPlayerSaveGame.h"
#include "Character/BasePlayerController.h"

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
			if (MasterVolumeText)
			{
				MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->MasterVolume * 100.f)));
			}
			MasterVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnMasterVolumeChanged);
		}
		if (BGMVolumeSlider)
		{
			BGMVolumeSlider->SetValue(OptionManager->BGMVolume);
			if (BGMVolumeText)
			{
				BGMVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->BGMVolume * 100.f)));
			}
			BGMVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnBGMVolumeChanged);
		}
		if (EffectVolumeSlider)
		{
			EffectVolumeSlider->SetValue(OptionManager->EffectVolume);
			if (EffectVolumeText)
			{
				EffectVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->EffectVolume * 100.f)));
			}
			EffectVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnEffectVolumeChanged);
		}
		if (MouseSensitivitySlider)
		{
			MouseSensitivitySlider->SetValue(OptionManager->MouseSensitivity);
			if (MouseSensitivityText)
			{
				MouseSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->MouseSensitivity * 100.f)));
			}
			MouseSensitivitySlider->OnValueChanged.AddUniqueDynamic(this, &UGeneralOptionWidget::OnSensitivityChanged);
		}
		if (BrightnessSlider)
		{
			BrightnessSlider->SetValue(OptionManager->Brightness);
			if (BrightnessText)
			{
				BrightnessText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->Brightness * 100.f)));
			}
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
	InitializeAllOptions();
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
	// LOG_Frame_WARNING(TEXT("Master Volume Changed"));

	if (MasterVolumeText)
	{
		MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), Value * 100.f)));
	}

	ULCLocalPlayerSaveGame::SaveMasterVolume(GetWorld(), Value);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MasterVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UGeneralOptionWidget::OnBGMVolumeChanged(float Value)
{
	// LOG_Frame_WARNING(TEXT("BGM Volume Changed"));

	if (BGMVolumeText)
	{
		BGMVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), Value * 100.f)));
	}

	ULCLocalPlayerSaveGame::SaveBGMVolume(GetWorld(), Value);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->BGMVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UGeneralOptionWidget::OnEffectVolumeChanged(float Value)
{
	// LOG_Frame_WARNING(TEXT("Effect Volume Changed"));

	if (EffectVolumeText)
	{
		EffectVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), Value * 100.f)));
	}

	ULCLocalPlayerSaveGame::SaveEffectVolume(GetWorld(), Value);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->EffectVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UGeneralOptionWidget::OnSensitivityChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("SenSitivity Changed"));
	const float MinValue = 0.01f;
	float AdjustedValue = FMath::Max(Value, MinValue);

	if (MouseSensitivityText)
	{
		MouseSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), AdjustedValue * 100.f)));
	}

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MouseSensitivity = AdjustedValue;
	}

	ULCLocalPlayerSaveGame::SaveMouseSensitivity(GetWorld(), AdjustedValue);

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ABasePlayerController* MyPC = Cast<ABasePlayerController>(PC))
		{
			MyPC->SetMouseSensitivity(AdjustedValue);
		}
	}
}

void UGeneralOptionWidget::OnBrightnessChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Brightness Changed"));

	if (BrightnessText)
	{
		BrightnessText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), Value * 100.f)));
	}

	ULCLocalPlayerSaveGame::SaveBrightness(GetWorld(), Value);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->Brightness = Value;
	}

	if (APlayerController* PC = GetWorld()->GetFirstPlayerController())
	{
		if (ABasePlayerController* MyPC = Cast<ABasePlayerController>(PC))
		{
			MyPC->SetBrightness(Value);
		}
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
		ULCLocalPlayerSaveGame::SaveFullScreenMode(GetWorld(), bIsFullscreen);
	}
}

void UGeneralOptionWidget::OnResolutionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	SelectedItem = SelectedItem.TrimStartAndEnd();
	LOG_Frame_WARNING(TEXT("Resolution Changed: %s"), *SelectedItem);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->ResolutionIndex = ResolutionComboBox->FindOptionIndex(SelectedItem);

		FString LeftStr, RightStr;
		if (SelectedItem.Split(TEXT("x"), &LeftStr, &RightStr))
		{
			int32 Width = FCString::Atoi(*LeftStr);
			int32 Height = FCString::Atoi(*RightStr);
			OptionManager->ScreenResolution = FIntPoint(Width, Height);

			UE_LOG(LogTemp, Warning, TEXT("Parsed Resolution: %d x %d"), Width, Height);
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to parse resolution string: %s"), *SelectedItem);
		}
	}
}

void UGeneralOptionWidget::PopulateResolutionOptions()
{
	if (!ResolutionComboBox)
	{
		return;
	}

	ResolutionComboBox->ClearOptions();

	FDisplayMetrics DisplayMetrics;
	FSlateApplication::Get().GetDisplayMetrics(DisplayMetrics);

	const int32 MaxWidth = DisplayMetrics.PrimaryDisplayWidth;
	const int32 MaxHeight = DisplayMetrics.PrimaryDisplayHeight;

	TArray<FIntPoint> CandidateResolutions = {
		{1280, 720}, {1366, 768}, {1600, 900},
		{1920, 1080}, {2560, 1440}, {3840, 2160}
	};

	FIntPoint CurrentRes = { 1920, 1080 };
	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		CurrentRes = OptionManager->ScreenResolution;
	}

	FString CurrentResStr = FString::Printf(TEXT("%d x %d"), CurrentRes.X, CurrentRes.Y);
	int32 FoundIndex = INDEX_NONE;

	for (int32 i = 0; i < CandidateResolutions.Num(); ++i)
	{
		const FIntPoint& Res = CandidateResolutions[i];
		if (Res.X <= MaxWidth && Res.Y <= MaxHeight)
		{
			FString ResStr = FString::Printf(TEXT("%d x %d"), Res.X, Res.Y);
			ResolutionComboBox->AddOption(ResStr);

			if (Res == CurrentRes)
			{
				FoundIndex = ResolutionComboBox->GetOptionCount() - 1;
			}
		}
	}

	if (FoundIndex != INDEX_NONE)
	{
		ResolutionComboBox->SetSelectedIndex(FoundIndex);
	}
	else
	{
		ResolutionComboBox->SetSelectedIndex(0);
	}
}

void UGeneralOptionWidget::InitializeSensitivity()
{
	if (UWorld* World = GetWorld())
	{
		float SavedSensitivity = ULCLocalPlayerSaveGame::LoadMouseSensitivity(World);

		if (MouseSensitivitySlider)
		{
			MouseSensitivitySlider->SetValue(SavedSensitivity);
		}

		if (MouseSensitivityText)
		{
			MouseSensitivityText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedSensitivity * 100.f)));
		}
	}
}

void UGeneralOptionWidget::InitializeMasterVolume() 
{
	if (UWorld* World = GetWorld())
	{
		float SavedMasterVolume= ULCLocalPlayerSaveGame::LoadMasterVolume(World);

		if (MasterVolumeSlider)
		{
			MasterVolumeSlider->SetValue(SavedMasterVolume);
		}

		if (MasterVolumeText)
		{
			MasterVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedMasterVolume * 100.f)));
		}
	}
}

void UGeneralOptionWidget::InitializeBGMVolume() 
{
	if (UWorld* World = GetWorld())
	{
		float SavedBGMVolume = ULCLocalPlayerSaveGame::LoadBGMVolume(World);

		if (BGMVolumeSlider)
		{
			BGMVolumeSlider->SetValue(SavedBGMVolume);
		}

		if (BGMVolumeText)
		{
			BGMVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedBGMVolume * 100.f)));
		}
	}
}

void UGeneralOptionWidget::InitializeEffectVolume() 
{
	if (UWorld* World = GetWorld())
	{
		float SavedEffectVolume = ULCLocalPlayerSaveGame::LoadEffectVolume(World);

		if (EffectVolumeSlider)
		{
			EffectVolumeSlider->SetValue(SavedEffectVolume);
		}

		if (EffectVolumeText)
		{
			EffectVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedEffectVolume * 100.f)));
		}
	}
}

void UGeneralOptionWidget::InitializeBrightness() 
{
	if (UWorld* World = GetWorld())
	{
		float SavedBrightness = ULCLocalPlayerSaveGame::LoadBrightness(World);

		if (BrightnessSlider)
		{
			BrightnessSlider->SetValue(SavedBrightness);
		}

		if (BrightnessText)
		{
			BrightnessText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedBrightness * 100.f)));
		}
	}
}

void UGeneralOptionWidget::InitializeScreenMode() 
{
	if (UWorld* World = GetWorld())
	{
		bool bIsFullScreenMode = ULCLocalPlayerSaveGame::LoadFullScreenMode(World);

		if (FullScreenCheckBox)
		{
			FullScreenCheckBox->SetIsChecked(bIsFullScreenMode);
		}
		if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
		{
			OptionManager->ChangeScreen(bIsFullScreenMode == true ? EScreenMode::FullScreen : EScreenMode::Window);
		}
	}

	
}

void UGeneralOptionWidget::InitializeAllOptions() 
{
	InitializeSensitivity();
	InitializeMasterVolume();
	InitializeBGMVolume();
	InitializeEffectVolume();
	InitializeBrightness();
	InitializeScreenMode();
}