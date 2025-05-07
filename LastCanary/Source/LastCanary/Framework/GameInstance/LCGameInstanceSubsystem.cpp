#include "LastCanary/Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"

void ULCGameInstanceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	LCUIManager = NewObject<ULCUIManager>(GetGameInstance());
}

ULCUIManager* ULCGameInstanceSubsystem::GetUIManager() const
{
	return LCUIManager;
}
