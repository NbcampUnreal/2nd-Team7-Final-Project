#include "UI/UIObject/VoiceOptionWidget.h"
#include "Framework/GameInstance/LCOptionManager.h"

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
	if (!InputModeText) return;

	FString ModeString;
	switch (CurrentInputMode)
	{
	case EVoiceInputMode::Toggle:  ModeString = TEXT("토글"); break;
	case EVoiceInputMode::Hold:    ModeString = TEXT("홀드"); break;
	case EVoiceInputMode::Always:  ModeString = TEXT("상시오픈"); break;
	case EVoiceInputMode::Off:     ModeString = TEXT("끄기"); break;
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

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->VoiceVolume = Value;
		OptionManager->ApplyAudio(); // 볼륨은 바로 적용
	}
}