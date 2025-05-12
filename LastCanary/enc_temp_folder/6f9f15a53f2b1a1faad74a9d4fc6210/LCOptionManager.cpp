#include "LCOptionManager.h"
#include "GameFramework/GameUserSettings.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

void ULCOptionManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	LOG_Frame_WARNING(TEXT("ULCOptionManager Initialized"));
}

void ULCOptionManager::Deinitialize()
{
	Super::Deinitialize();
	LOG_Frame_WARNING(TEXT("ULCOptionManager Deinitialized"));
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

	UGameplayStatics::PushSoundMixModifier(this, AudioMix);
}

void ULCOptionManager::ApplyOptions()
{
	if (UGameUserSettings* Settings = GEngine->GetGameUserSettings())
	{
		UE_LOG(LogTemp, Warning, TEXT("APPLYING SETTINGS"));
		UE_LOG(LogTemp, Warning, TEXT("ScreenResolution: %d x %d"), ScreenResolution.X, ScreenResolution.Y);
		UE_LOG(LogTemp, Warning, TEXT("ScreenMode: %s"), ScreenMode == EScreenMode::FullScreen ? TEXT("Fullscreen") : TEXT("Windowed"));

		Settings->SetScreenResolution(ScreenResolution);
		Settings->SetFullscreenMode(ScreenMode == EScreenMode::FullScreen ? EWindowMode::Fullscreen : EWindowMode::Windowed);
		Settings->ApplySettings(false);

		// TODO: 마우스 감도, 밝기 적용
		// 마우스 감도, 밝기는 별도 시스템 적용 필요
		// 감도는 컨트롤러에 반영, 밝기는 PostProcess에 적용 등

		LOG_Frame_WARNING(TEXT("Options Applied: %d x %d, Mode: %s"),
			ScreenResolution.X, ScreenResolution.Y,
			ScreenMode == EScreenMode::FullScreen ? TEXT("Fullscreen") : TEXT("Windowed"));
	}
}
