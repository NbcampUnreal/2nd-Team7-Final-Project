#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCUIManagerSettings.h"

#include "UI/UIElement/TitleMenu.h"
#include "UI/UIElement/LobbyMenu.h"
#include "UI/UIElement/OptionWidget.h"
#include "UI/UIElement/PauseMenu.h"
#include "UI/UIElement/InGameHUD.h"
#include "UI/UIElement/ShopWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/Popup/PopupCreateSession.h"
#include "UI/Popup/PopupNotice.h"
#include "UI/Popup/PopupLoading.h"
#include "UI/Popup/NotePopupWidget.h"
#include "UI/UIElement/LoadingLevel.h"
#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIElement/ResultMenu.h"
#include "UI/UIElement/RoomWidget.h"
#include "UI/UIElement/DroneHUD.h"
#include "UI/UIElement/SpectatorWidget.h"
#include "UI/UIElement/GameOverWidget.h"

#include "UI/UIObject/ConfirmPopup.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "LastCanary.h"

ULCUIManager::ULCUIManager()
{
	CurrentWidget = nullptr;
}

void ULCUIManager::InitUIManager(APlayerController* PlayerController)
{
	if (OwningPlayer == nullptr)
	{
		OwningPlayer = PlayerController;
	}

	if (const ULCGameInstance* GI = Cast<ULCGameInstance>(PlayerController->GetGameInstance()))
	{
		if (const ULCUIManagerSettings* Settings = GI->GetUIManagerSettings())
		{
			TitleMenuClass = Settings->FromBPTitleMenuClass;
			LobbyMenuClass = Settings->FromBPLobbyMenuClass;
			OptionWidgetClass = Settings->FromBPOptionWidgetClass;
			PauseMenuClass = Settings->FromBPPauseMenuClass;
			InGameHUDWidgetClass = Settings->FromBPInGameHUDClass;
			ShopWidgetClass = Settings->FromBPShopWidgetClass;
			InventoryMainWidgetClass = Settings->FromBPInventoryMainUIClass;
			CreateSessionClass = Settings->FromBPCreateSessionWidgetClass;
			PopUpNoticeClass = Settings->FromBPPopupNoticeClass;
			PopUpLoadingClass = Settings->FromBPPopupLoadingClass;
			LoadingLevelClass = Settings->FromBPLoadingLevelClass;
			ConfirmPopupClass = Settings->FromBPConfirmPopupClass;
			ChecklistWidgetClass = Settings->FromBPChecklistWidgetClass;
			ResultMenuClass = Settings->FromBPResultMenuClass;
			RoomWidgetClass = Settings->FromBPRoomWidgetClass;
			NotePopupWidgetClass = Settings->FromBPNotePopupWidgetClass;
			DroneHUDClass = Settings->FromBPDroneHUDClass;
			SpectatorWidgetClass = Settings->FromBPSpectatorWidgetClass;
			GameOverWidgetClass = Settings->FromBPGameOverWidgetClass;

			if ((CachedTitleMenu == nullptr) && TitleMenuClass)
			{
				CachedTitleMenu = CreateWidget<UTitleMenu>(PlayerController, TitleMenuClass);
			}
			if ((CachedLobbyMenu == nullptr) && LobbyMenuClass)
			{
				CachedLobbyMenu = CreateWidget<ULobbyMenu>(PlayerController, LobbyMenuClass);
			}
			if ((CachedOptionWidget == nullptr) && OptionWidgetClass)
			{
				CachedOptionWidget = CreateWidget<UOptionWidget>(PlayerController, OptionWidgetClass);
			}
			if ((CachedPauseMenu == nullptr) && PauseMenuClass)
			{
				CachedPauseMenu = CreateWidget<UPauseMenu>(PlayerController, PauseMenuClass);
			}
			if ((CachedInGameHUD == nullptr) && InGameHUDWidgetClass)
			{
				CachedInGameHUD = CreateWidget<UInGameHUD>(PlayerController, InGameHUDWidgetClass);
			}
			if ((CachedShopWidget == nullptr) && ShopWidgetClass)
			{
				CachedShopWidget = CreateWidget<UShopWidget>(PlayerController, ShopWidgetClass);
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
			if ((CachedChecklistWidget == nullptr) && ChecklistWidgetClass)
			{
				CachedChecklistWidget = CreateWidget<UChecklistWidget>(PlayerController, ChecklistWidgetClass);
			}
			if ((CachedResultMenu == nullptr) && ResultMenuClass)
			{
				CachedResultMenu = CreateWidget<UResultMenu>(PlayerController, ResultMenuClass);
			}
			if ((CachedRoomWidget == nullptr) && RoomWidgetClass)
			{
				CachedRoomWidget = CreateWidget<URoomWidget>(PlayerController, RoomWidgetClass);
				CachedRoomWidget->CreatePlayerSlots();
			}
			if ((CachedNotePopupWidget == nullptr) && NotePopupWidgetClass)
			{
				CachedNotePopupWidget = CreateWidget<UNotePopupWidget>(PlayerController, NotePopupWidgetClass);
			}
			if ((CachedDroneHUD == nullptr) && DroneHUDClass)
			{
				CachedDroneHUD = CreateWidget<UDroneHUD>(PlayerController, DroneHUDClass);
			}
			if ((CachedSpectatorWidget == nullptr) && SpectatorWidgetClass)
			{
				CachedSpectatorWidget = CreateWidget<USpectatorWidget>(PlayerController, SpectatorWidgetClass);
			}
			if ((CachedGameOverWidget == nullptr) && GameOverWidgetClass)
			{
				CachedGameOverWidget = CreateWidget<UGameOverWidget>(PlayerController, GameOverWidgetClass);
			}
		}
	}

	if (bSessionErrorOccurred)
	{
		ShowPopupNotice(CachedErrorReson);
		bSessionErrorOccurred = false;
	}
}

void ULCUIManager::SetPlayerController(APlayerController* PlayerController)
{
	UE_LOG(LogTemp, Warning, TEXT("ULCUIManager::SetPlayerController - PlayerController: %s"), PlayerController ? *PlayerController->GetName() : TEXT("nullptr"));
	OwningPlayer = PlayerController;
}

void ULCUIManager::ShowTitleMenu()
{
	if (OwningPlayer == nullptr)
	{
		LOG_Frame_WARNING(TEXT("OwningPlayer is nullptr"));
		return;
	}

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

void ULCUIManager::ShowOptionWidget()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (IsValid(CachedOptionWidget) == false)
	{
		CachedOptionWidget = CreateWidget<UOptionWidget>(GetWorld(), OptionWidgetClass);
	}

	switch (CurrentContext)
	{
	case ELCUIContext::Title:
		if (CachedOptionWidget->IsInViewport() == false)
		{
			CachedOptionWidget->AddToViewport(5);
		}
		SetInputModeUIOnly(CachedOptionWidget);
		break;
	case ELCUIContext::Room:
		// falls through
	case ELCUIContext::InGame:
		HideInGameHUD();
		SwitchToWidget(CachedOptionWidget);
		SetInputModeUIOnly(CachedOptionWidget);
		break;
	default:
		SwitchToWidget(CachedOptionWidget);
		SetInputModeUIOnly(CachedOptionWidget);
		break;
	}
}

void ULCUIManager::HideOptionWidget()
{
	if (OwningPlayer == nullptr || OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (CachedOptionWidget && CachedOptionWidget->IsInViewport())
	{
		CachedOptionWidget->RemoveFromParent();
	}

	switch (CurrentContext)
	{
	case ELCUIContext::Title:
		ShowTitleMenu();
		break;
	case ELCUIContext::Room:
		// falls through
	case ELCUIContext::InGame:
		ShowPauseMenu();
		break;
	default:
		ShowInGameHUD();
		break;
	}
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
	if (IsValid(CachedPauseMenu) == false)
	{
		return;
	}

	HideInGameHUD();
	SwitchToWidget(CachedPauseMenu);
	SetInputModeUIOnly(CachedPauseMenu);
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
	if (IsValid(CachedPauseMenu) && CachedPauseMenu->IsInViewport())
	{
		CachedPauseMenu->RemoveFromParent();
	}
	ShowInGameHUD();
	SetInputModeGameOnly();
}

bool ULCUIManager::IsPauseMenuOpen() const
{
	return CachedPauseMenu && CachedPauseMenu->IsInViewport();
}

void ULCUIManager::ShowConfirmPopup(TFunction<void()> OnConfirm, const FText& Message)
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
	if (ConfirmPopupClass == nullptr)
	{
		return;
	}

	UConfirmPopup* ConfirmPopup = CreateWidget<UConfirmPopup>(OwningPlayer, ConfirmPopupClass);
	if (ConfirmPopup)
	{
		ConfirmPopup->Init(MoveTemp(OnConfirm), Message);
		ConfirmPopup->AddToViewport(10);
	}
}

void ULCUIManager::ShowShopPopup(int Gold)
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
	{
		LastShopInteractor->GetShopWidgetComponent()->SetVisibility(false);
	}

	SwitchToWidget(CachedShopWidget);
	CachedShopWidget->SetGold(Gold);
	HideInventoryMainWidget();

	if (APawn* Pawn = OwningPlayer->GetPawn())
	{
		Pawn->DisableInput(OwningPlayer);
	}
	SetInputModeUIOnly(CachedShopWidget);
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
	if (LastShopInteractor && LastShopInteractor->GetShopWidgetComponent())
	{
		LastShopInteractor->GetShopWidgetComponent()->SetVisibility(true);
	}

	SwitchToWidget(CachedInGameHUD);
	ShowInventoryMainWidget();
	SetInputModeGameOnly();

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
}

void ULCUIManager::ShowCreateSession()
{
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

void ULCUIManager::ShowChecklistWidget()
{
	if (OwningPlayer == nullptr)
	{
		APlayerController* PC = GetWorld()->GetFirstPlayerController();
		if (PC && PC->IsLocalController())
		{
			SetPlayerController(PC);
			LOG_Frame_WARNING(TEXT("UIManager: OwningPlayer를 복구함 -> %s"), *PC->GetName());
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("ShowChecklistWidget: OwningPlayer = %s"), *OwningPlayer->GetName());

	SwitchToWidget(CachedChecklistWidget);
	HideInventoryMainWidget();
	HideSpectatorWidget();
	SetInputModeUIOnly(CachedChecklistWidget);
}

UResultMenu* ULCUIManager::ShowResultMenu()
{
	if (!CachedResultMenu && ResultMenuClass)
	{
		CachedResultMenu = CreateWidget<UResultMenu>(OwningPlayer, ResultMenuClass);
	}

	if (CachedResultMenu && !CachedResultMenu->IsInViewport())
	{
		CachedResultMenu->AddToViewport(999);
	}

	SetInputModeUIOnly(CachedResultMenu);
	return CachedResultMenu;
}

void ULCUIManager::ShowRoomWidget()
{
	if (OwningPlayer == nullptr || OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (CachedRoomWidget)
	{
		SwitchToWidget(CachedRoomWidget);
	}
}

void ULCUIManager::HideRoomWidget()
{
	if (CachedRoomWidget)
	{
		if (CachedRoomWidget->IsInViewport())
		{
			CachedRoomWidget->RemoveFromParent();
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("HideRoomWidget: CachedRoomWidget is nullptr"));
	}
}

void ULCUIManager::ShowDroneHUD()
{
	if (CachedDroneHUD)
	{
		if (!CachedDroneHUD->IsInViewport())
		{
			CachedDroneHUD->AddToViewport(1);
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("ShowDroneHUD: CachedDroneHUD is nullptr"));
	}
}

void ULCUIManager::HideDroneHUD()
{
	if (CachedDroneHUD)
	{
		if (CachedDroneHUD->IsInViewport())
		{
			CachedDroneHUD->RemoveFromParent();
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("HideDroneHUD: CachedDroneHUD is nullptr"));
	}
}

void ULCUIManager::ShowSpectatorWidget()
{
	if (CachedSpectatorWidget)
	{
		if (!CachedSpectatorWidget->IsInViewport())
		{
			CachedSpectatorWidget->AddToViewport(1);
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("ShowSpectatorWidget: CachedSpectatorWidget is nullptr"));
	}
}

void ULCUIManager::HideSpectatorWidget()
{
	if (CachedSpectatorWidget)
	{
		if (CachedSpectatorWidget->IsInViewport())
		{
			CachedSpectatorWidget->RemoveFromParent();
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("HideSpectatorWidget: CachedSpectatorWidget is nullptr"));
	}
}

void ULCUIManager::ShowGameOverWidget()
{
	if (!OwningPlayer || !OwningPlayer->IsLocalPlayerController()) return;

	// 1. 모든 위젯 제거
	if (CurrentWidget && CurrentWidget->IsInViewport())
	{
		CurrentWidget->RemoveFromParent();
	}
	HideInventoryMainWidget();
	HidePauseMenu();
	HideRoomWidget();
	HideDroneHUD();
	HideSpectatorWidget();
	HideNotePopup();
	HidePopUpNotice();
	HidePopUpLoading();
	HideLoadingLevel();

	// 2. GameOverWidget 표시
	if (CachedGameOverWidget)
	{
		CachedGameOverWidget->AddToViewport(999); // 가장 위
	}

	// 3. 입력 모드 설정
	SetInputModeUIOnly(CachedGameOverWidget);
}

void ULCUIManager::HideGameOverWidget()
{
	if (CachedGameOverWidget && CachedGameOverWidget->IsInViewport())
	{
		CachedGameOverWidget->RemoveFromParent();
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

void ULCUIManager::ShowPopupNotice(const FText& Notice)
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

void ULCUIManager::ShowNotePopup(const FText& NoteText, const TArray<TSoftObjectPtr<UTexture2D>>& CandidateImages, int32 SelectedIndex)
{
	if (NotePopupWidgetClass == nullptr)
	{
		return;
	}

	if (CachedNotePopupWidget == nullptr)
	{
		CachedNotePopupWidget = CreateWidget<UNotePopupWidget>(GetWorld(), NotePopupWidgetClass);
		if (CachedNotePopupWidget == nullptr)
		{
			return;
		}
	}

	if (CachedNotePopupWidget->IsInViewport() == false)
	{
		CachedNotePopupWidget->AddToViewport(50);
		CachedNotePopupWidget->SetKeyboardFocus();
	}

	CachedNotePopupWidget->ShowNoteContent(NoteText, CandidateImages, SelectedIndex);
	SetInputModeGameAndUI();
}

void ULCUIManager::HideNotePopup()
{
	if (CachedNotePopupWidget && CachedNotePopupWidget->IsInViewport())
	{
		CachedNotePopupWidget->RemoveFromParent();
	}

	SetInputModeGameOnly();
}

void ULCUIManager::ShowInGameHUD()
{
	if (OwningPlayer == nullptr || OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	SwitchToWidget(CachedInGameHUD);
	SetInputModeGameOnly();
	ShowInventoryMainWidget();
}

void ULCUIManager::HideInGameHUD()
{
	if (OwningPlayer == nullptr || OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
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

	if (FocusWidget)
	{
		LOG_Frame_WARNING(TEXT("SetInputModeUIOnly: %s"), *GetNameSafe(FocusWidget ? FocusWidget : CurrentWidget));
		LOG_Frame_WARNING(TEXT("SetWidgetFocus: %s"), *FocusWidget->GetName());
	}
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

void ULCUIManager::SetInputModeGameAndUI()
{
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		FInputModeGameAndUI InputMode;
		InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
		InputMode.SetHideCursorDuringCapture(false);
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void ULCUIManager::SetLastShopInteractor(AShopInteractor* Interactor)
{
	LastShopInteractor = Interactor;
}

void ULCUIManager::UpdateInputModeByContext()
{
	if (OwningPlayer == nullptr)
	{
		LOG_Frame_WARNING(TEXT("OwningPlayer is nullptr in SetInputModeByContext."));
		return;
	}

	switch (CurrentContext)
	{
	case ELCUIContext::Title:
		SetInputModeUIOnly(CachedTitleMenu);
		break;
	case ELCUIContext::Room:
		SetInputModeGameOnly();
		break;
	case ELCUIContext::InGame:
		SetInputModeGameOnly();
		break;
	default:
		SetInputModeGameOnly();
		break;
	}
}

void ULCUIManager::SetUIContext(ELCUIContext NewContext)
{
	CurrentContext = NewContext;
}

const ELCUIContext ULCUIManager::GetUIContext() const
{
	return CurrentContext;
}

void ULCUIManager::SetSessionErrorState(const FText& Reason)
{
	bSessionErrorOccurred = true;
	CachedErrorReson = Reason;
}
