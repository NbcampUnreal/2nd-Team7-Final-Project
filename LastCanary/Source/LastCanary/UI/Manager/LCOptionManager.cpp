#include "UI/Manager/LCOptionManager.h"
#include "GameFramework/GameUserSettings.h"
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

void ULCOptionManager::ApplyOptions()
{
	if (GEngine)
	{
		UGameUserSettings* Settings = GEngine->GetGameUserSettings();

		Settings->SetScreenResolution(ScreenResolution);
		Settings->ApplySettings(false);

		LOG_Frame_WARNING(TEXT("Options Applied: Resolution %d x %d"), ScreenResolution.X, ScreenResolution.Y);
	}
}