#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/GameMode/LCRoomGameMode.h"

#include "UI/UIElement/RoomWidget.h"

#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Misc/PackageName.h"

#include "UI/Manager/LCUIManager.h"
#include "Blueprint/UserWidget.h"

#include "OnlineSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"


void ALCRoomPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// TODO : Base Camp Level 에서 인터렉션 후 UI 표시
	CreateAndShowSelecetGameUI();
	CreateAndShowRoomUI();

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->InitUIManager(this);
			UIManager->ShowInGameHUD();
		}
	}
}

void ALCRoomPlayerController::Client_UpdatePlayers_Implementation()
{
	Super::Client_UpdatePlayers_Implementation();
	FTimerHandle TimerHandle;
	FTimerDelegate TimerDel;
	TimerDel.BindLambda([this]()
		{
			if (RoomWidgetInstance)
			{
				UE_LOG(LogTemp, Warning, TEXT("Update Lobby UI!!"));
				RoomWidgetInstance->UpdatePlayerNames();
			}
		});

	GetWorld()->GetTimerManager().SetTimer(TimerHandle, TimerDel, 1.0f, false);
}

void ALCRoomPlayerController::StartGame()
{
	if (!HasAuthority()) return;

	if (UWorld* World = GetWorld())
	{
		if (ALCRoomGameMode* RoomGM = Cast<ALCRoomGameMode>(World->GetAuthGameMode()))
		{
			RoomGM->StartGame();
		}
	}
}

void ALCRoomPlayerController::CreateAndShowSelecetGameUI()
{
	if (HasAuthority() && IsLocalPlayerController())
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
}

void ALCRoomPlayerController::CreateAndShowRoomUI()
{
	if (IsLocalPlayerController())
	{
		if (RoomWidgetClass)
		{
			RoomWidgetInstance = CreateWidget<URoomWidget>(this, RoomWidgetClass);
			if (RoomWidgetInstance)
			{
				RoomWidgetInstance->AddToViewport();
			}
		}
	}
}
