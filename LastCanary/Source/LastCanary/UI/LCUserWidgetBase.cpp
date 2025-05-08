#include "LastCanary/UI/LCUserWidgetBase.h"
#include "LastCanary/Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

ULCUIManager* ULCUserWidgetBase::ResolveUIManager() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const ULCGameInstanceSubsystem* Subsystem = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			return Subsystem->GetUIManager();
		}
	}
	LOG_Frame_WARNING(TEXT("ResolveUIManager failed : GameInstance or UI Subsystem is null"));
	return nullptr;
}
