#include "UI/UIObject/VoiceOptionWidget.h"
#include "Framework/GameInstance/LCOptionManager.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"

#include "Components/Slider.h"
#include "Components/TextBlock.h"
#include "Components/ComboBoxString.h"
#include "Components/Button.h"

#include "LastCanary.h"



void UVoiceOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>();

	if (MyMicVolumeSlider)
	{
		MyMicVolumeSlider->SetValue(OptionManager->MyMicVolume);
		if (MyMicVolumeText)
		{
			MyMicVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->MyMicVolume * 100.f)));
		}
		MyMicVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UVoiceOptionWidget::OnMicVolumeChanged);
	}
	if (VoiceVolumeSlider)
	{
		VoiceVolumeSlider->SetValue(OptionManager->VoiceVolume);
		if (VoiceVolumeText)
		{
			VoiceVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), OptionManager->VoiceVolume * 100.f)));
		}
		VoiceVolumeSlider->OnValueChanged.AddUniqueDynamic(this, &UVoiceOptionWidget::OnVoiceVolumeChanged);
	}

	RefreshInputModeText();
	InitializeAllOptions();
}

void UVoiceOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (MyMicVolumeSlider)
	{
		MyMicVolumeSlider->OnValueChanged.RemoveDynamic(this, &UVoiceOptionWidget::OnMicVolumeChanged);
	}
	if (VoiceVolumeSlider)
	{
		VoiceVolumeSlider->OnValueChanged.RemoveDynamic(this, &UVoiceOptionWidget::OnVoiceVolumeChanged);
	}
}

void UVoiceOptionWidget::RefreshInputModeText()
{
	if (InputModeText == nullptr)
	{
		return;
	}

	FString ModeString;
	switch (CurrentInputMode)
	{
	case EVoiceInputMode::Toggle:
		ModeString = TEXT("Toggle to Talk");
		break;
	case EVoiceInputMode::Hold:
		ModeString = TEXT("Hold to Talk");
		break;
	case EVoiceInputMode::Always:
		ModeString = TEXT("Open Mic Always");
		break;
	case EVoiceInputMode::Off:
		ModeString = TEXT("Off");
		break;
	}
	InputModeText->SetText(FText::FromString(ModeString));
}

void UVoiceOptionWidget::OnMicVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("MyMic Volume Changed"));

	if (MyMicVolumeText)
	{
		MyMicVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), Value * 100.f)));
	}

	ULCLocalPlayerSaveGame::SaveMicrophoneVolume(GetWorld(), Value);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->MyMicVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UVoiceOptionWidget::OnVoiceVolumeChanged(float Value)
{
	LOG_Frame_WARNING(TEXT("Voice Volume Changed"));

	if (VoiceVolumeText)
	{
		VoiceVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), Value * 100.f)));
	}

	ULCLocalPlayerSaveGame::SaveVoiceChatVolume(GetWorld(), Value);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->VoiceVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}

void UVoiceOptionWidget::InitializeVoiceChatVolume()
{
	if (UWorld* World = GetWorld())
	{
		float SavedVoiceChatVolume = ULCLocalPlayerSaveGame::LoadVoiceChatVolume(World);

		if (VoiceVolumeSlider)
		{
			VoiceVolumeSlider->SetValue(SavedVoiceChatVolume);
		}

		if (VoiceVolumeText)
		{
			VoiceVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedVoiceChatVolume * 100.f)));
		}
	}
}

void UVoiceOptionWidget::InitializeMicrophoneVolume()
{
	if (UWorld* World = GetWorld())
	{
		float SavedMicrophoneVolume = ULCLocalPlayerSaveGame::LoadMicrophoneVolume(World);

		if (MyMicVolumeSlider)
		{
			MyMicVolumeSlider->SetValue(SavedMicrophoneVolume);
		}

		if (MyMicVolumeText)
		{
			MyMicVolumeText->SetText(FText::FromString(FString::Printf(TEXT("%.0f %%"), SavedMicrophoneVolume * 100.f)));
		}
	}
}

void UVoiceOptionWidget::InitializeAllOptions()
{
	InitializeVoiceChatVolume();
	InitializeMicrophoneVolume();
}