#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCUIManagerSettings.h"

#include "UI/UIElement/TitleMenu.h"
#include "UI/UIElement/LobbyMenu.h"
#include "UI/UIElement/EnterPasswordWidget.h"
#include "UI/UIElement/OptionWidget.h"
#include "UI/UIElement/InGameHUD.h"

#include "Framework/PlayerController/LCLobbyPlayerController.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

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
			OptionWidgetClass = Settings->OptionWidgetClass;
			InGameHUDWidgetClass = Settings->InGameHUDClass;

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
			if ((CachedOptionWidget == nullptr) && OptionWidgetClass)
			{
				CachedOptionWidget = CreateWidget<UOptionWidget>(PlayerController, OptionWidgetClass);
			}
			if ((CachedInGameHUD == nullptr) && InGameHUDWidgetClass)
			{
				CachedInGameHUD = CreateWidget<UInGameHUD>(PlayerController, InGameHUDWidgetClass);
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
	LOG_Frame_WARNING(TEXT("ShowTitleMenu"));
	SwitchToWidget(CachedTitleMenu);
}

void ULCUIManager::ShowLobbyMenu()
{
	LOG_Frame_WARNING(TEXT("ShowLobbyMenu"));
	SwitchToWidget(CachedLobbyMenu);
}

void ULCUIManager::ShowRoomListMenu()
{
	LOG_Frame_WARNING(TEXT("ShowRoomListMenu"));
}

void ULCUIManager::ShowEnterPasswordWidget(const FString& RoomID)
{
	LOG_Frame_WARNING(TEXT("ShowEnterPasswordWidget"));
	if (CachedEnterPasswordWidget)
	{
		CachedEnterPasswordWidget->Init(RoomID);
	}
	SwitchToWidget(CachedEnterPasswordWidget);
}

void ULCUIManager::ShowOptionPopup()
{
	LOG_Frame_WARNING(TEXT("ShowOptionPopup"));

	if (CachedOptionWidget && CachedOptionWidget->IsInViewport() == false)
	{
		CachedOptionWidget->AddToViewport(1); 
	}

	if (OwningPlayer)
	{
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(CachedOptionWidget->TakeWidget()); 
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->bShowMouseCursor = true;
	}
}

void ULCUIManager::ShowInGameHUD()
{
	LOG_Frame_WARNING(TEXT("ShowInGameHUD"));
	SwitchToWidget(CachedInGameHUD);
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
			CurrentWidget->RemoveFromParent();
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
