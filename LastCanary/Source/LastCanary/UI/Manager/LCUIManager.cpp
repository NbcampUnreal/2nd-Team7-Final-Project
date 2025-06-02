#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCUIManagerSettings.h"

#include "UI/UIElement/TitleMenu.h"
#include "UI/UIElement/LobbyMenu.h"
#include "UI/UIElement/EnterPasswordWidget.h"
#include "UI/UIElement/OptionWidget.h"
#include "UI/UIElement/InGameHUD.h"
#include "UI/UIElement/ShopWidget.h"
#include "UI/UIElement/MapSelectWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/Popup/PopupCreateSession.h"
#include "UI/Popup/PopupNotice.h"
#include "UI/Popup/PopupLoading.h"
#include "UI/UIElement/LoadingLevel.h"

#include "UI/UIObject/ConfirmPopup.h"

//#include "Framework/PlayerController/LCLobbyPlayerController.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Components/WidgetComponent.h"

#include "LastCanary.h"

ULCUIManager::ULCUIManager()
{
	CurrentWidget = nullptr;
}

void ULCUIManager::InitUIManager(APlayerController* PlayerController)
{
	//LOG_Frame_WARNING(TEXT("InitUIManager Called."));
	if (OwningPlayer == nullptr)
	{
		//LOG_Frame_WARNING(TEXT("PlayerController is nullptr in InitUIManager."));
		OwningPlayer = PlayerController;
	}

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
			MapSelectWidgetClass = Settings->FromBPMapSelectWidgetClass;
			InventoryMainWidgetClass = Settings->FromBPInventoryMainUIClass;
			CreateSessionClass = Settings->FromBPCreateSessionWidgetClass;
			PopUpNoticeClass = Settings->FromBPPopupNoticeClass;
			PopUpLoadingClass = Settings->FromBPPopupLoadingClass;
			LoadingLevelClass = Settings->FromBPLoadingLevelClass;
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
			}
			if ((CachedMapSelectWidget == nullptr) && MapSelectWidgetClass)
			{
				CachedMapSelectWidget = CreateWidget<UMapSelectWidget>(PlayerController, MapSelectWidgetClass);
			}
			if ((CachedPopupLoading == nullptr) && PopUpLoadingClass)
			{
				CachedPopupLoading = CreateWidget<UPopupLoading>(PlayerController, PopUpLoadingClass);
			}
			if ((CachedPopupNotice == nullptr) && PopUpNoticeClass)
			{
				CachedPopupNotice = CreateWidget<UPopupNotice>(PlayerController, PopUpNoticeClass);
			}
			if ((CachedLoadingLevel == nullptr) && LoadingLevelClass)
			{
				CachedLoadingLevel = CreateWidget<ULoadingLevel>(PlayerController, LoadingLevelClass);
			}
			if ((CachedInventoryMainWidget == nullptr) && InventoryMainWidgetClass)
			{
				CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(PlayerController, InventoryMainWidgetClass);
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
	if (OwningPlayer == nullptr)
	{
		LOG_Frame_WARNING(TEXT("OwningPlayer is nullptr"));
		return;
	}
	/*if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}*/

	LOG_Frame_WARNING(TEXT("ShowTitleMenu"));
	SwitchToWidget(CachedTitleMenu);
	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::ShowLobbyMenu()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("ShowLobbyMenu"));
	SwitchToWidget(CachedLobbyMenu);
	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::ShowRoomListMenu()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("ShowRoomListMenu"));
}

void ULCUIManager::ShowEnterPasswordWidget(const FString& RoomID)
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("ShowEnterPasswordWidget"));
	if (CachedEnterPasswordWidget)
	{
		CachedEnterPasswordWidget->Init(RoomID);
	}
	SwitchToWidget(CachedEnterPasswordWidget);
	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::ShowOptionPopup()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("ShowOptionPopup"));

	if (!IsValid(CachedOptionWidget))
	{
		CachedOptionWidget = CreateWidget<UOptionWidget>(GetWorld(), OptionWidgetClass);
	}

	if (CachedOptionWidget && CachedOptionWidget->IsInViewport() == false)
	{
		CachedOptionWidget->AddToViewport(1);
	}

	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::ShowPauseMenu()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

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
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	SetInputModeGameOnly();
}

void ULCUIManager::ShowConfirmPopup(TFunction<void()> OnConfirm)
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

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
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("ShowShopPopup"));
	// HideInGameHUD();

	if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
	{
		LastShopInteractor->GetShopWidgetComponent()->SetVisibility(false);
	}
	/*if (CachedShopWidget)
	{
		CachedShopWidget->AddToViewport(1);
	}*/

	SwitchToWidget(CachedShopWidget);
	HideInventoryMainWidget();

	if (OwningPlayer)
	{
		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			LOG_Frame_WARNING(TEXT("Pawn : %s"), *Pawn->GetActorNameOrLabel());

			Pawn->DisableInput(OwningPlayer);
		}

		FInputModeUIOnly InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		LOG_Frame_WARNING(TEXT("InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock"));
	}

	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::HideShopPopup()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("HideShopPopup Called"));

	if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
	{
		LastShopInteractor->GetShopWidgetComponent()->SetVisibility(true);
	}

	/*if (CachedShopWidget)
	{
		CachedShopWidget->RemoveFromParent();
	}*/

	if (OwningPlayer)
	{
		LOG_Frame_WARNING(TEXT("OwningPlayer Exist %s"), *OwningPlayer->GetActorNameOrLabel());

		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			LOG_Frame_WARNING(TEXT("Pawn Exist : %s"), *Pawn->GetActorNameOrLabel());

			Pawn->EnableInput(OwningPlayer);
		}

		OwningPlayer->SetViewTargetWithBlend(OwningPlayer->GetPawn(), 1.0f);
	}
	else
	{
		LOG_Frame_WARNING(TEXT("OwningPlayer is nullptr"));
	}

	SetInputModeGameOnly();
	//ShowInGameHUD();
	SwitchToWidget(CachedInGameHUD);
	ShowInventoryMainWidget();
}

void ULCUIManager::ShowCreateSession()
{
	LOG_Frame_WARNING(TEXT("Show Create Session"));
	if (CreateSessionClass)
	{
		UPopupCreateSession* CreateSessionWidget = CreateWidget<UPopupCreateSession>(OwningPlayer, CreateSessionClass);
		CreateSessionWidget->AddToViewport(1);
	}
}

void ULCUIManager::ToggleInventory()
{
	if (!CachedInventoryMainWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[ToggleInventory] 인벤토리 위젯이 없습니다. InitUIManager가 제대로 호출되지 않았을 수 있습니다."));
		return;
	}

	if (!CachedInventoryMainWidget->IsInViewport())
	{
		CachedInventoryMainWidget->AddToViewport(1);
		CachedInventoryMainWidget->ShowToolbarOnly();
	}

	CachedInventoryMainWidget->ToggleBackpackInventory();

	if (OwningPlayer)
	{
		if (CachedInventoryMainWidget->IsBackpackInventoryOpen())
		{
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(CachedInventoryMainWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			OwningPlayer->SetInputMode(InputMode);
			OwningPlayer->bShowMouseCursor = true;
		}
		else
		{
			OwningPlayer->SetInputMode(FInputModeGameOnly());
			OwningPlayer->bShowMouseCursor = false;
		}
	}
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

void ULCUIManager::ShowPopupNotice(FString Notice)
{
	LOG_Frame_WARNING(TEXT("Show Popup Notice"));
	if (CachedPopupNotice)
	{
		CachedPopupNotice->InitializeNoticePopup(Notice);
		CachedPopupNotice->AddToViewport(1);
	}
}

void ULCUIManager::HidePopUpNotice()
{
	if (CachedPopupNotice)
	{
		CachedPopupNotice->RemoveFromParent();
	}
}

void ULCUIManager::ShowLoadingLevel()
{
	if (CachedLoadingLevel)
	{
		CachedLoadingLevel->AddToViewport(10);
	}
}

void ULCUIManager::HideLoadingLevel()
{
	if (CachedLoadingLevel)
	{
		CachedLoadingLevel->RemoveFromParent();
	}
}

void ULCUIManager::ShowMapSelectPopup()
{
	LOG_Frame_WARNING(TEXT("ShowMapSelectPopup"));
	HideInGameHUD();

	if (LastMapSelectInteractor && LastMapSelectInteractor->GetMapSelectWidgetComponent())
	{
		LastMapSelectInteractor->GetMapSelectWidgetComponent()->SetVisibility(false);
	}

	if (CachedMapSelectWidget && LastMapSelectInteractor)
	{
		CachedMapSelectWidget->AddToViewport(1);
		CachedMapSelectWidget->GateActorInstance = LastMapSelectInteractor->TargetGateActor;
	}

	if (OwningPlayer)
	{
		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			Pawn->DisableInput(OwningPlayer);
		}
	}
	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::HideMapSelectPopup()
{
	LOG_Frame_WARNING(TEXT("HideMapSelectPopup"));

	if (LastMapSelectInteractor && LastMapSelectInteractor->GetMapSelectWidgetComponent())
	{
		LastMapSelectInteractor->GetMapSelectWidgetComponent()->SetVisibility(true);
	}

	if (CachedMapSelectWidget)
	{
		CachedMapSelectWidget->RemoveFromParent();
	}

	if (OwningPlayer)
	{
		if (APawn* Pawn = OwningPlayer->GetPawn())
		{
			Pawn->EnableInput(OwningPlayer);
		}

		OwningPlayer->SetViewTargetWithBlend(OwningPlayer->GetPawn(), 1.0f);
	}
	SetInputModeGameOnly();
	ShowInGameHUD();
}

void ULCUIManager::ShowInGameHUD()
{
	//LOG_Frame_WARNING(TEXT("ShowInGameHUD"));
	SwitchToWidget(CachedInGameHUD);
	SetInputModeGameOnly();

	ShowInventoryMainWidget();
}

void ULCUIManager::HideInGameHUD()
{
	//LOG_Frame_WARNING(TEXT("HideInGameHUD"));
	if (CachedInGameHUD && CachedInGameHUD->IsInViewport())
	{
		CachedInGameHUD->RemoveFromParent();
	}

	HideInventoryMainWidget();
}

void ULCUIManager::ShowInventoryMainWidget()
{
	if (CachedInventoryMainWidget)
	{
		if (!CachedInventoryMainWidget->IsInViewport())
		{
			CachedInventoryMainWidget->AddToViewport(1);
		}
		CachedInventoryMainWidget->ShowToolbarOnly();
	}
}

void ULCUIManager::HideInventoryMainWidget()
{
	if (CachedInventoryMainWidget && CachedInventoryMainWidget->IsInViewport())
	{
		CachedInventoryMainWidget->RemoveFromParent();
	}
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

void ULCUIManager::SetLastMapSelectInteractor(AMapSelectInteractor* Interactor)
{
	LastMapSelectInteractor = Interactor;
}
