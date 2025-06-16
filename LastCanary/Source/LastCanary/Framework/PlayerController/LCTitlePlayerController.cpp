#include "Framework/PlayerController/LCTitlePlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerState.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

void ALCTitlePlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	bShowMouseCursor = true;

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->InitUIManager(this);
			UIManager->ShowTitleMenu();
			UIManager->SetUIContext(ELCUIContext::Title);
		}
	}
}