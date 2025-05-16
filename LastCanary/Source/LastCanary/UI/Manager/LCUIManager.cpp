#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCUIManagerSettings.h"

#include "UI/UIElement/TitleMenu.h"
#include "UI/UIElement/LobbyMenu.h"
#include "UI/UIElement/EnterPasswordWidget.h"
#include "UI/UIElement/OptionWidget.h"
#include "UI/UIElement/InGameHUD.h"
#include "UI/UIElement/ShopWidget.h"

#include "Framework/PlayerController/LCLobbyPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Components/WidgetComponent.h"

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
			TitleMenuClass = Settings->FromBPTitleMenuClass;
			LobbyMenuClass = Settings->FromBPLobbyMenuClass;
			EnterPasswordWidgetClass = Settings->FromBPEnterPasswordWidgetClass;
			OptionWidgetClass = Settings->FromBPOptionWidgetClass;
			InGameHUDWidgetClass = Settings->FromBPInGameHUDClass;
			ShopWidgetClass = Settings->FromBPShopWidgetClass;

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
			if ((CachedShopWidget == nullptr) && ShopWidgetClass)
			{
				CachedShopWidget = CreateWidget<UShopWidget>(PlayerController, ShopWidgetClass);
				LOG_Frame_WARNING(TEXT("CachedShopWidget Created"));
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

void ULCUIManager::ShowShopPopup()
{
	LOG_Frame_WARNING(TEXT("ShowShopPopup"));

	if (!ShopWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShopWidgetClass is null"));
		return;
	}

	if (!CachedShopWidget)
	{
		CachedShopWidget = CreateWidget<UShopWidget>(OwningPlayer, ShopWidgetClass);
	}

	if (CachedShopWidget && !CachedShopWidget->IsInViewport())
	{
		CachedShopWidget->AddToViewport();

		OwningPlayer->SetInputMode(FInputModeUIOnly());
		OwningPlayer->bShowMouseCursor = true;

		if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
		{
			LastShopInteractor->GetShopWidgetComponent()->SetVisibility(false);
		}
	}
}

void ULCUIManager::HideShopPopup()
{
	LOG_Frame_WARNING(TEXT("HideShopPopup"));

	if (CachedShopWidget && CachedShopWidget->IsInViewport())
	{
		CachedShopWidget->RemoveFromParent();
	}

	OwningPlayer->SetInputMode(FInputModeGameOnly());
	OwningPlayer->bShowMouseCursor = false;

	if (APawn* ControlledPawn = OwningPlayer->GetPawn())
	{
		OwningPlayer->SetViewTargetWithBlend(ControlledPawn, 0.5f);
	}

	if (LastShopInteractor)
	{
		if (LastShopInteractor->GetShopWidgetComponent())
		{
			LastShopInteractor->GetShopWidgetComponent()->SetVisibility(true);
		}
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

void ULCUIManager::SetLastShopInteractor(AShopInteractor* Interactor)
{
	LastShopInteractor = Interactor;
}
