#include "Framework/PlayerController/LCLobbyPlayerController.h"
#include "Framework/GameMode/LCLobbyGameMode.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "UI/Manager/LCUIManager.h"

void ALCLobbyPlayerController::BeginPlay()
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
			UIManager->ShowLobbyMenu();
		}
	}
}

void ALCLobbyPlayerController::Server_RequestJoinRoom_Implementation(const FString& RoomID, const FString& Password)
{
    if (ALCLobbyGameMode* GM = GetWorld()->GetAuthGameMode<ALCLobbyGameMode>())
    {
        GM->TryJoinRoom(this, RoomID, Password);
    }
}
