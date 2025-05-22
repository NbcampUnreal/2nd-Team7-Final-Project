#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCUIManagerSettings.h"

#include "UI/UIElement/TitleMenu.h"
#include "UI/UIElement/LobbyMenu.h"
#include "UI/UIElement/EnterPasswordWidget.h"
#include "UI/UIElement/OptionWidget.h"
#include "UI/UIElement/InGameHUD.h"
#include "UI/UIElement/ShopWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/UIElementCreateSession.h"

#include "UI/UIObject/ConfirmPopup.h"
#include "UI/Popup/PopupLoading.h"

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
			InventoryMainWidgetClass = Settings->FromBPShopWidgetClass;
			CreateSessionClass = Settings->FromBPCreateSessionWidgetClass;
			PopUpLoadingClass = Settings->FromBPPopupLoadingClass;

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

			if ((CachedInventoryMainWidget == nullptr) && InventoryMainWidgetClass)
			{
				CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(PlayerController, InventoryMainWidgetClass);
			}
			if ((CachedCreateSession == nullptr) && CreateSessionClass)
			{
				CachedCreateSession = CreateWidget<UUIElementCreateSession>(PlayerController, CreateSessionClass);
			}
			if ((CachedPopupLoading == nullptr) && PopUpLoadingClass)
			{
				CachedPopupLoading = CreateWidget<UPopupLoading>(PlayerController, PopUpLoadingClass);
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

	SetInputModeUIOnly(CachedOptionWidget);
}

void ULCUIManager::ShowPauseMenu()
{
	LOG_Frame_WARNING(TEXT("ShowPauseMenu"));
	if (CachedOptionWidget)
	{
		CachedOptionWidget->AddToViewport(1);
	}

	if (OwningPlayer)
	{
		SetInputModeUIOnly(CachedOptionWidget);
	}
}

void ULCUIManager::HidePauseMenu()
{
	SetInputModeGameOnly();
}

void ULCUIManager::ShowConfirmPopup(TFunction<void()> OnConfirm)
{
	LOG_Frame_WARNING(TEXT("ShowConfirmPopup"));
	if (!ConfirmPopupClass)
	{
		return;
	}

	UConfirmPopup* ConfirmPopup = CreateWidget<UConfirmPopup>(OwningPlayer, ConfirmPopupClass);
	if (ConfirmPopup)
	{
		ConfirmPopup->Init(MoveTemp(OnConfirm));
		ConfirmPopup->AddToViewport(10);
	}
}

void ULCUIManager::ShowShopPopup()
{
	LOG_Frame_WARNING(TEXT("ShowShopPopup"));

	if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
	{
		LastShopInteractor->GetShopWidgetComponent()->SetVisibility(false);
	}

	if (CachedShopWidget)
	{
		CachedShopWidget->AddToViewport(1);
	}

	if (OwningPlayer)
	{
		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			Pawn->DisableInput(OwningPlayer);
		}

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->bShowMouseCursor = true;
	}
}

// 사용하지 않게 될 확률이 높음
//void ULCUIManager::ShowInventoryPopup()
//{
//	if (!CachedInventoryMainWidget && InventoryMainWidgetClass)
//	{
//		CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(OwningPlayer, InventoryMainWidgetClass);
//	}
//
//	if (CachedInventoryMainWidget)
//	{
//		if (!CachedInventoryMainWidget->IsInViewport())
//		{
//			CachedInventoryMainWidget->AddToViewport();
//		}
//		else
//		{
//			CachedInventoryMainWidget->SetVisibility(ESlateVisibility::Visible);
//		}
//
//		if (OwningPlayer)
//		{
//			FInputModeGameAndUI InputMode;
//			InputMode.SetWidgetToFocus(CachedInventoryMainWidget->TakeWidget());
//			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
//			OwningPlayer->SetInputMode(InputMode);
//			OwningPlayer->bShowMouseCursor = true;
//		}
//	}
//
//}

void ULCUIManager::ToggleInventoryPopup()
{
	if (!CachedInventoryMainWidget && InventoryMainWidgetClass)
	{
		CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(OwningPlayer, InventoryMainWidgetClass);
	}

	if (CachedInventoryMainWidget)
	{
		if (CachedInventoryMainWidget->IsInViewport() && CachedInventoryMainWidget->GetVisibility() == ESlateVisibility::Visible)
		{
			CachedInventoryMainWidget->SetVisibility(ESlateVisibility::Collapsed);

			if (OwningPlayer)
			{
				OwningPlayer->SetInputMode(FInputModeGameOnly());
				OwningPlayer->bShowMouseCursor = false;
			}
		}
		else
		{
			if (!CachedInventoryMainWidget->IsInViewport())
			{
				CachedInventoryMainWidget->AddToViewport();
			}

			CachedInventoryMainWidget->SetVisibility(ESlateVisibility::Visible);

			if (OwningPlayer)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(CachedInventoryMainWidget->TakeWidget());

				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				OwningPlayer->SetInputMode(InputMode);
				OwningPlayer->bShowMouseCursor = true;
			}
		}
	}
}

void ULCUIManager::HideShopPopup()
{
	LOG_Frame_WARNING(TEXT("HideShopPopup"));

	if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
	{
		LastShopInteractor->GetShopWidgetComponent()->SetVisibility(true);
	}

	if (CachedShopWidget)
	{
		CachedShopWidget->RemoveFromParent();
	}

	if (OwningPlayer)
	{
		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			Pawn->EnableInput(OwningPlayer);
		}

		OwningPlayer->SetViewTargetWithBlend(OwningPlayer->GetPawn(), 1.0f);
		OwningPlayer->SetInputMode(FInputModeGameOnly());
		OwningPlayer->bShowMouseCursor = false;
	}
}

void ULCUIManager::ShowCreateSession()
{
	LOG_Frame_WARNING(TEXT("Show Create Session"));
	if (CreateSessionClass)
	{
		CachedCreateSession = CreateWidget<UUIElementCreateSession>(OwningPlayer, CreateSessionClass);
		CachedCreateSession->AddToViewport(1);
	}
	//SwitchToWidget(CachedCreateSession);
}

void ULCUIManager::ShowPopUpLoading()
{
	if (CachedPopupLoading)
	{
		CachedPopupLoading->AddToViewport(1);
	}
}

void ULCUIManager::HidePopUpLoading()
{
	if (CachedPopupLoading)
	{
		CachedPopupLoading->RemoveFromParent();
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
	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::SetInputModeUIOnly(UUserWidget* FocusWidget)
{
	if (OwningPlayer)
	{
		FInputModeUIOnly InputMode;
		if (FocusWidget)
		{
			InputMode.SetWidgetToFocus(FocusWidget->TakeWidget());
		}
		else if (CurrentWidget)
		{
			InputMode.SetWidgetToFocus(CurrentWidget->TakeWidget());
		}
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->bShowMouseCursor = true;
	}

	LOG_Frame_WARNING(TEXT("SetInputModeUIOnly: %s"), *GetNameSafe(FocusWidget ? FocusWidget : CurrentWidget));
}

void ULCUIManager::SetInputModeGameOnly()
{
	if (OwningPlayer)
	{
		FInputModeGameOnly InputMode;
		OwningPlayer->SetInputMode(InputMode);
		OwningPlayer->bShowMouseCursor = false;
	}
}

void ULCUIManager::SetLastShopInteractor(AShopInteractor* Interactor)
{
	LastShopInteractor = Interactor;
}
