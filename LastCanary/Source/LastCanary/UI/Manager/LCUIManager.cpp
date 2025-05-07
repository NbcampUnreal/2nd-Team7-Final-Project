#include "LastCanary/UI/Manager/LCUIManager.h"
#include "LastCanary/UI/Manager/LCUIManagerSettings.h"

#include "LastCanary/UI/UIElement/TitleMenu.h"
#include "LastCanary/UI/UIElement/LobbyMenu.h"
#include "LastCanary/UI/UIElement/EnterPasswordWidget.h"

#include "LastCanary/Framework/PlayerController/LCLobbyPlayerController.h"

#include "LastCanary/Framework/GameInstance/LCGameInstance.h"
#include "LastCanary/Framework/GameInstance/LCGameInstanceSubsystem.h"
ULCUIManager::ULCUIManager()
{
	CurrentWidget = nullptr;
	// Constructor logic here
}

void ULCUIManager::InitUIManager(APlayerController* PlayerController)
{
	OwningPlayer = PlayerController;
	if (const ULCGameInstance* GI = Cast<ULCGameInstance>(PlayerController->GetGameInstance()))
	{
		if (const ULCUIManagerSettings* Settings = GI->GetUIManagerSettings())
		{
			TitleMenuClass = Settings->TitleMenuClass;
			LobbyMenuClass = Settings->LobbyMenuClass;
			EnterPasswordWidgetClass = Settings->EnterPasswordWidgetClass;

			if ((CachedTitleMenu == nullptr) && TitleMenuClass)
			{
				CachedTitleMenu = CreateWidget<UTitleMenu>(PlayerController, TitleMenuClass);
			}
			if ((CachedLobbyMenu == nullptr) && LobbyMenuClass)
			{
				CachedLobbyMenu = CreateWidget<ULobbyMenu>(PlayerController, LobbyMenuClass);
			}
			if ((CachedEnterPasswordWidget == nullptr) && EnterPasswordWidgetClass)
			{
				CachedEnterPasswordWidget = CreateWidget<UEnterPasswordWidget>(PlayerController, EnterPasswordWidgetClass);
			}
		}
	}
}

void ULCUIManager::SetPlayerController(APlayerController* PlayerController)
{
	OwningPlayer = PlayerController;
}

void ULCUIManager::ShowTitleMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowTitleMenu"));
	SwitchToWidget(CachedTitleMenu);
}

void ULCUIManager::ShowLobbyMenu()
{
	UE_LOG(LogTemp, Warning, TEXT("ShowLobbyMenu"));
	SwitchToWidget(CachedLobbyMenu);
}

void ULCUIManager::ShowEnterPasswordWidget(const FString& RoomID)
{
	UE_LOG(LogTemp, Warning, TEXT("ShowEnterPasswordWidget"));
	if (CachedEnterPasswordWidget)
	{
		CachedEnterPasswordWidget->Init(RoomID);
	}
	SwitchToWidget(CachedEnterPasswordWidget);
}

void ULCUIManager::SwitchToWidget(UUserWidget* NewWidget)
{
	if (NewWidget == nullptr)
	{
		return;
	}

	if (CurrentWidget && CurrentWidget != NewWidget)
	{
		if (CurrentWidget->IsInViewport())
		{
			CurrentWidget->RemoveFromViewport();
		}
	}

	if (NewWidget->IsInViewport() == false)
	{
		NewWidget->AddToViewport();
	}
	else
	{
		NewWidget->SetVisibility(ESlateVisibility::Visible);
	}

	CurrentWidget = NewWidget;

	if (OwningPlayer)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->bShowMouseCursor = true;
	}
}
