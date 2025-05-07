#include "LastCanary/UI/LCUserWidgetBase.h"
#include "LastCanary/Framework/GameInstance/LCGameInstanceSubsystem.h"

ULCUIManager* ULCUserWidgetBase::ResolveUIManager() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const ULCGameInstanceSubsystem* Subsystem = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			return Subsystem->GetUIManager();
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("ResolveUIManager failed: GameInstance or UI Subsystem was null"));
	return nullptr;
}
