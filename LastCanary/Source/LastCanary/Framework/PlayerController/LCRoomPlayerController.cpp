#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameMode/LCRoomGameMode.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"

#include "UI/Manager/LCUIManager.h"
#include "Blueprint/UserWidget.h"

void ALCRoomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->InitUIManager(this);
		}
	}

	if (IsLocalPlayerController())
	{
		CreateAndShowSelecetGameUI();
	}
}

void ALCRoomPlayerController::CreateAndShowSelecetGameUI()
{
	if (StartGameWidgetClass)
	{
		UUserWidget* StartWidget = CreateWidget<UUserWidget>(this, StartGameWidgetClass);
		if (StartWidget)
		{
			StartWidget->AddToViewport();
		}
	}
}

void ALCRoomPlayerController::Server_StartGame_Implementation()
{
	if (UWorld* W = GetWorld())
	{
		if (ALCRoomGameMode* RoomGM = Cast<ALCRoomGameMode>(W->GetAuthGameMode()))
		{
			RoomGM->StartGame();
		}
	}
}
