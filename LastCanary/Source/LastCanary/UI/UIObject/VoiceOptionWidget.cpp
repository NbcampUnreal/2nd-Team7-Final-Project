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
	EnumerateInputDevices();

	if (MicDeviceComboBox)
	{
		MicDeviceComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UVoiceOptionWidget::OnMicDeviceChanged);
	}

	if (VoiceInputModeButtonPre)
	{
		VoiceInputModeButtonPre->OnClicked.AddUniqueDynamic(this, &UVoiceOptionWidget::OnPreVoiceInputModeChanged);
	}
	if (VoiceInputModeButtonNext)
	{
		VoiceInputModeButtonNext->OnClicked.AddUniqueDynamic(this, &UVoiceOptionWidget::OnNextVoiceInputModeChanged);
	}

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

	if (MicDeviceComboBox)
	{
		MicDeviceComboBox->OnSelectionChanged.RemoveDynamic(this, &UVoiceOptionWidget::OnMicDeviceChanged);
	}
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

void UVoiceOptionWidget::EnumerateInputDevices()
{
	//TODO : 여기에 마이크 장치 변경 로직을 추가해야 합니다.
	if (!MicDeviceComboBox) return;

	MicDeviceComboBox->ClearOptions();

	// 임시 더미 장치 목록
	MicDeviceComboBox->AddOption(TEXT("Default Microphone"));
	MicDeviceComboBox->AddOption(TEXT("External USB Mic"));
	MicDeviceComboBox->AddOption(TEXT("Virtual Cable Input"));

	MicDeviceComboBox->SetSelectedIndex(0);
}

void UVoiceOptionWidget::OnMicDeviceChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	if (SelectionType == ESelectInfo::Direct) return; // 프로그래밍적으로 선택된 경우 무시

	LOG_Frame_WARNING(TEXT("Mic Device Changed to: %s"), *SelectedItem);

	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->SelectedMicDeviceName = SelectedItem;
		// OptionManager->ApplyVoiceInputDevice(); // 필요 시 추가
	}
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

void UVoiceOptionWidget::OnPreVoiceInputModeChanged()
{
	int32 Prev = ((int32)CurrentInputMode - 1 + 4) % 4; // 음수 방지
	CurrentInputMode = static_cast<EVoiceInputMode>(Prev);
	RefreshInputModeText();
}

void UVoiceOptionWidget::OnNextVoiceInputModeChanged()
{
	int32 Next = ((int32)CurrentInputMode + 1) % 4;
	CurrentInputMode = static_cast<EVoiceInputMode>(Next);
	RefreshInputModeText();
}