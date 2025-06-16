#include "LCOptionManager.h"
#include "GameFramework/GameUserSettings.h"
#include "Framework/GameInstance/LCOptionSettingAsset.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/GameEngine.h"
#include "Framework/GameInstance/LCGameInstance.h"

#include "LastCanary.h"

void ULCOptionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ULCGameInstance* GI = GetWorld()->GetGameInstance<ULCGameInstance>();
	if (!GI)
	{
		return;
	}
	else
	{
		SoundClassSettings = GI->GetOptionSettings();
	}

	if (SoundClassSettings)
	{
		AudioMix = SoundClassSettings->AudioMix;
		MasterSoundClass = SoundClassSettings->MasterSoundClass;
		BGMSoundClass = SoundClassSettings->BGMSoundClass;
		EffectSoundClass = SoundClassSettings->EffectSoundClass;
		VoiceSoundClass = SoundClassSettings->VoiceSoundClass;
		MyMicSoundClass = SoundClassSettings->MyMicSoundClass;
	}
}

void ULCOptionManager::Deinitialize()
{
	Super::Deinitialize();
}

void ULCOptionManager::ChangeScreen(EScreenMode Mode)
{
	if (ScreenMode != Mode)
	{
		ScreenMode = Mode;
		switch (ScreenMode)
		{
		case EScreenMode::FullScreen:
		{
			GEngine->GetGameUserSettings()->SetFullscreenMode(EWindowMode::Fullscreen);
			LOG_Frame_WARNING(TEXT("FullScreen"));
			break;
		}
		case EScreenMode::Window:
		{
			GEngine->GetGameUserSettings()->SetFullscreenMode(EWindowMode::Windowed);
			LOG_Frame_WARNING(TEXT("Window"));
			break;
		}
		default:
			break;
		}
	}
}

void ULCOptionManager::ApplyAudio()
{
	if (!AudioMix || !MasterSoundClass || !BGMSoundClass || !EffectSoundClass)
	{
		return;
	}

	UGameplayStatics::SetSoundMixClassOverride(this, AudioMix, MasterSoundClass, MasterVolume);
	UGameplayStatics::SetSoundMixClassOverride(this, AudioMix, BGMSoundClass, BGMVolume);
	UGameplayStatics::SetSoundMixClassOverride(this, AudioMix, EffectSoundClass, EffectVolume);
	UGameplayStatics::SetSoundMixClassOverride(this, AudioMix, VoiceSoundClass, VoiceVolume);

	UGameplayStatics::PushSoundMixModifier(this, AudioMix);
}

void ULCOptionManager::ApplyOptions()
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		Settings->SaveSettings();/*
		UE_LOG(LogTemp, Warning, TEXT("APPLYING SETTINGS"));
		UE_LOG(LogTemp, Warning, TEXT("ScreenResolution: %d x %d"), ScreenResolution.X, ScreenResolution.Y);
		UE_LOG(LogTemp, Warning, TEXT("ScreenMode: %s"), ScreenMode == EScreenMode::FullScreen ? TEXT("Fullscreen") : TEXT("Windowed"));

		Settings->SetScreenResolution(ScreenResolution);
		Settings->SetFullscreenMode(ScreenMode == EScreenMode::FullScreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		Settings->ApplySettings(false);
		FSlateApplication::Get().Tick();

		if (GEngine && GEngine->GameViewport)
		{
			TSharedPtr<SViewport> ViewportWidget = GEngine->GameViewport->GetGameViewportWidget();
			if (ViewportWidget.IsValid())
			{
				ViewportWidget->Invalidate(EInvalidateWidget::LayoutAndVolatility);
			}
		}

		// TODO: 마우스 감도, 밝기 적용
		// 마우스 감도, 밝기는 별도 시스템 적용 필요
		// 감도는 컨트롤러에 반영, 밝기는 PostProcess에 적용 등

		LOG_Frame_WARNING(TEXT("Options Applied: %d x %d, Mode: %s"),
			ScreenResolution.X, ScreenResolution.Y,
			ScreenMode == EScreenMode::FullScreen ? TEXT("Fullscreen") : TEXT("Windowed"));
			*/
	}
}
