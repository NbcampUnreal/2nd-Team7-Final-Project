#include "Framework/GameInstance/LCGameInstance.h"
#include "UI/Manager/LCUIManagerSettings.h"

ULCUIManagerSettings* ULCGameInstance::GetUIManagerSettings() const
{
	if (UIManagerSettings)
	{
		return UIManagerSettings;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("UIManagerSettings is not set in the GameInstance."));
		return nullptr;
	}
}