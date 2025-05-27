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

	if (IsValid(LCUIManager))
	{
		UE_LOG(LogTemp, Warning, TEXT("TitlePC"));
		LCUIManager->ShowTitleMenu();
	}

	const FString PlayerName = PlayerState->GetPlayerName();
	PlayerState->SetPlayerName(PlayerName);
}