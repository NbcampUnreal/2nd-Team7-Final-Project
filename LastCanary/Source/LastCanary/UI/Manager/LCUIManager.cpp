#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCUIManagerSettings.h"

#include "UI/UIElement/TitleMenu.h"
#include "UI/UIElement/LobbyMenu.h"
#include "UI/UIElement/OptionWidget.h"
#include "UI/UIElement/PauseMenu.h"
#include "UI/UIElement/InGameHUD.h"
#include "UI/UIElement/ShopWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/LoadingLevel.h"
#include "UI/UIElement/ChecklistWidget.h"
#include "UI/UIElement/ResultMenu.h"
#include "UI/UIElement/ResultWidget.h"
#include "UI/UIElement/RoomWidget.h"
#include "UI/UIElement/DroneHUD.h"
#include "UI/UIElement/SpectatorWidget.h"
#include "UI/UIElement/GameOverWidget.h"
#include "UI/UIElement/GameEndWidget.h"
#include "UI/UIElement/ServerMessageWidget.h"
#include "UI/UIElement/DesktopWidget.h"
#include "UI/UIObject/TaskbarWidget.h"

#include "UI/Popup/PopupCreateSession.h"
#include "UI/Popup/PopupNotice.h"
#include "UI/Popup/PopupLoading.h"
#include "UI/Popup/NotePopupWidget.h"

#include "UI/UIObject/ConfirmPopup.h"

#include "UI/Manager/LCDesktopWindowManager.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "Components/WidgetComponent.h"
#include "Character/BaseSpectatorPawn.h"
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

	if (!PlayerController || !PlayerController->IsLocalPlayerController())
	{
		UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] %s 는 로컬 컨트롤러가 아님"), *GetNameSafe(PlayerController));
		return;
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
			ResultWidgetClass = Settings->FromBPResultWidgetClass;
			RoomWidgetClass = Settings->FromBPRoomWidgetClass;
			NotePopupWidgetClass = Settings->FromBPNotePopupWidgetClass;
			DroneHUDClass = Settings->FromBPDroneHUDClass;
			SpectatorWidgetClass = Settings->FromBPSpectatorWidgetClass;
			GameOverWidgetClass = Settings->FromBPGameOverWidgetClass;
			GameEndWidgetClass = Settings->FromBPGameEndWidgetClass;
			ServerMessageWidgetClass = Settings->FromBPServerMessageWidgetClass;
			DesktopWidgetClass = Settings->FromBPDesktopWidgetClass;

			CreateAndCacheWidget(CachedTitleMenu, TitleMenuClass);
			CreateAndCacheWidget(CachedLobbyMenu, LobbyMenuClass);
			CreateAndCacheWidget(CachedOptionWidget, OptionWidgetClass);
			CreateAndCacheWidget(CachedPauseMenu, PauseMenuClass);
			CreateAndCacheWidget(CachedInGameHUD, InGameHUDWidgetClass);
			CreateAndCacheWidget(CachedShopWidget, ShopWidgetClass);
			CreateAndCacheWidget(CachedPopupLoading, PopUpLoadingClass);
			CreateAndCacheWidget(CachedPopupNotice, PopUpNoticeClass);
			CreateAndCacheWidget(CachedLoadingLevel, LoadingLevelClass);
			CreateAndCacheWidget(CachedInventoryMainWidget, InventoryMainWidgetClass);
			CreateAndCacheWidget(CachedChecklistWidget, ChecklistWidgetClass);
			CreateAndCacheWidget(CachedResultMenu, ResultMenuClass);
			CreateAndCacheWidget(CachedRoomWidget, RoomWidgetClass);
			CreateAndCacheWidget(CachedNotePopupWidget, NotePopupWidgetClass);
			CreateAndCacheWidget(CachedDroneHUD, DroneHUDClass);
			CreateAndCacheWidget(CachedSpectatorWidget, SpectatorWidgetClass);
			CreateAndCacheWidget(CachedGameOverWidget, GameOverWidgetClass);
			CreateAndCacheWidget(CachedGameEndWidget, GameEndWidgetClass);
			CreateAndCacheWidget(CachedServerMessageWidget, ServerMessageWidgetClass);

			if (CachedRoomWidget)
			{
				CachedRoomWidget->CreatePlayerSlots();
			}
			if (CachedServerMessageWidget)
			{
				CachedServerMessageWidget->AddToViewport();
			}
			if ((CachedDesktopWidget == nullptr) && DesktopWidgetClass)
			{
				CachedDesktopWidget = CreateWidget<UDesktopWidget>(PlayerController, DesktopWidgetClass);
				if (!CachedDesktopWidget)
				{
					LOG_Frame_WARNING(TEXT("Failed to create DesktopWidget"));
				}
				DesktopWindowManager = NewObject<ULCDesktopWindowManager>(this);
				if (!DesktopWindowManager)
				{
					LOG_Frame_WARNING(TEXT("Failed to create DesktopWindowManager"));
				}
				else
				{
					DesktopWindowManager->SetDesktopWidget(CachedDesktopWidget);
					CachedDesktopWidget->SetWindowManager(DesktopWindowManager);
					LOG_Frame_WARNING(TEXT("DesktopWidget successfully set to DesktopWindowManager"));
				}
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
	if (!PlayerController->IsLocalPlayerController())
	{
		return;
	}
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

void ULCUIManager::ChangeHUD()
{
	if (OwningPlayer == nullptr || !OwningPlayer->IsLocalPlayerController())
	{
		return;
	}

	//기존 HUD를 제거
	HideHUD();

	//디폴트를 InGame으로 하고 변수가 있으면 드론, 관전으로 이동
	switch (CurrentContext)
	{
	case ELCUIContext::DroneHUD:
		SwitchToWidget(CachedDroneHUD);
		break;

	case ELCUIContext::Spectator:
		SwitchToWidget(CachedSpectatorWidget);
		break;

	default:
		LOG_Frame_ERROR(TEXT("ChangeHUD: Invalid context %d"), (int32)CurrentContext);
		SwitchToWidget(CachedInGameHUD);
		ShowInventoryMainWidget();
		break;
	}
	SetInputModeGameOnly();
}

void ULCUIManager::HideHUD()
{
	if (OwningPlayer == nullptr || !OwningPlayer->IsLocalPlayerController())
	{
		return;
	}

	if (CachedDroneHUD && CachedDroneHUD->IsInViewport())
	{
		CachedDroneHUD->RemoveFromParent();
	}

	if (CachedSpectatorWidget && CachedSpectatorWidget->IsInViewport())
	{
		CachedSpectatorWidget->RemoveFromParent();
	}
	if (CachedInGameHUD && CachedInGameHUD->IsInViewport())
	{
		CachedInGameHUD->RemoveFromParent();
		HideInventoryMainWidget();
	}
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

	if (CachedOptionWidget->IsInViewport() == false)
	{
		CachedOptionWidget->AddToViewport(5);
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
		//case ELCUIContext::Room:
		//	// falls through
		//case ELCUIContext::InGame:
		//	ShowPauseMenu();
		//	break;
	default:
		ChangeHUD();
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

	HideHUD();
	SwitchToWidget(CachedPauseMenu);
	SetInputModeUIOnly(CachedPauseMenu);
}

void ULCUIManager::HidePauseMenu()
{
	if (OwningPlayer == nullptr || !OwningPlayer->IsLocalPlayerController())
	{
		return;
	}
	if (IsValid(CachedPauseMenu) && CachedPauseMenu->IsInViewport())
	{
		CachedPauseMenu->RemoveFromParent();
	}

	ChangeHUD();
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
	if (LastShopInteractor && LastShopInteractor->GetDesktopWidgetComponent())
	{
		LastShopInteractor->GetDesktopWidgetComponent()->SetVisibility(false);
	}

	HideHUD();
	SwitchToWidget(CachedShopWidget);
	CachedShopWidget->SetGold(Gold);
	//HideInventoryMainWidget();

	if (APawn* Pawn = OwningPlayer->GetPawn())
	{
		Pawn->DisableInput(OwningPlayer);
	}
	SetInputModeUIOnly(CachedShopWidget);
}

void ULCUIManager::HideShopPopup()
{
	if (CachedShopWidget && CachedShopWidget->IsInViewport())
	{
		CachedShopWidget->RemoveFromParent(); // 상점만 숨김
	}

	// 데스크탑 다시 표시
	ShowDesktop();
	ChangeHUD();
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

	HideHUD();
	SwitchToWidget(CachedChecklistWidget);
	//HideInventoryMainWidget();
	//HideSpectatorWidget();
	SetInputModeUIOnly(CachedChecklistWidget);
}

void ULCUIManager::ShowNewChecklistWidget(UDataTable* CheckListTable)
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

	HideHUD();
	SwitchToWidget(CachedChecklistWidget);
	//HideInventoryMainWidget();
	//HideSpectatorWidget();
	SetInputModeUIOnly(CachedChecklistWidget);

	CachedChecklistWidget->InitWithCheckListTable(CheckListTable);
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

UResultWidget* ULCUIManager::ShowResultWidget()
{
	if (!CachedResultWidget && ResultWidgetClass)
	{
		CachedResultWidget = CreateWidget<UResultWidget>(OwningPlayer, ResultWidgetClass);
	}

	if (CachedResultWidget && !CachedResultWidget->IsInViewport())
	{
		CachedResultWidget->AddToViewport(999);
	}

	SetInputModeUIOnly(CachedResultWidget);
	return CachedResultWidget;
}

void ULCUIManager::ShowRoomWidget()
{
	if (OwningPlayer == nullptr || OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (CachedRoomWidget)
	{
		HideHUD();
		SwitchToWidget(CachedRoomWidget);
		//HideInventoryMainWidget();
		SetInputModeUIOnly(CachedRoomWidget);
	}
}

void ULCUIManager::HideRoomWidget()
{
	if (CachedRoomWidget)
	{
		if (CachedRoomWidget->IsInViewport())
		{
			CachedRoomWidget->RemoveFromParent();

			ChangeHUD();
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("HideRoomWidget: CachedRoomWidget is nullptr"));
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
	//HideInventoryMainWidget();
	HideHUD();
	HidePauseMenu();
	HideRoomWidget();
	//HideDroneHUD();
	//HideSpectatorWidget();
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

void ULCUIManager::ShowGameEndWidget()
{
	if (CachedGameEndWidget)
	{
		if (!CachedGameEndWidget->IsInViewport())
		{
			CachedGameEndWidget->AddToViewport(999);
		}
	}
	else
	{
		LOG_Frame_ERROR(TEXT("ShowSpectatorWidget: CachedSpectatorWidget is nullptr"));
	}
}

void ULCUIManager::ShowHideEndWidget()
{
	if (CachedGameEndWidget && CachedGameEndWidget->IsInViewport())
	{
		CachedGameEndWidget->RemoveFromParent();
	}
}

void ULCUIManager::AddServerMessage(const FString& Message)
{
	if (CachedServerMessageWidget)
	{
		if (!CachedServerMessageWidget->IsInViewport())
		{
			CachedServerMessageWidget->AddToViewport();
			//CachedServerMessageWidget->AddMessage(Message);
		}
		CachedServerMessageWidget->AddMessage(Message);
	}
}

void ULCUIManager::ShowDesktop()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (LastShopInteractor && LastShopInteractor->GetDesktopWidgetComponent())
	{
		LastShopInteractor->GetDesktopWidgetComponent()->SetVisibility(false);
	}

	SwitchToWidget(CachedDesktopWidget);
	HideInventoryMainWidget();

	if (APawn* Pawn = OwningPlayer->GetPawn())
	{
		Pawn->DisableInput(OwningPlayer);
	}
	SetInputModeUIOnly(CachedDesktopWidget);
}

void ULCUIManager::HideDesktop()
{
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}
	if (LastShopInteractor && LastShopInteractor->GetDesktopWidgetComponent())
	{
		LastShopInteractor->GetDesktopWidgetComponent()->SetVisibility(true);
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
		OwningPlayer->SetViewTargetWithBlend(OwningPlayer->GetPawn(), 0.5f);
	}
	else
	{
		LOG_Frame_WARNING(TEXT("OwningPlayer is nullptr"));
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
	if (OwningPlayer == nullptr)
	{
		return;
	}
	if (OwningPlayer->IsLocalPlayerController() == false)
	{
		return;
	}

	if (CachedLoadingLevel)
	{
		CachedLoadingLevel->AddToViewport(10);

		//if (!CachedLoadingLevel->IsRooted())
		//{
		//	CachedLoadingLevel->AddToRoot();// GC 방지
		//}

		FTimerHandle ForceHideHandle;
		GetWorld()->GetTimerManager().SetTimer(ForceHideHandle, [this]()
			{
				LOG_Frame_WARNING(TEXT("Failsafe: 강제로 HideLoadingLevel 호출"));
				this->HideLoadingLevel();
			}, 10.0f, false);
	}
}

void ULCUIManager::RestoreLoadingScreenIfNeeded()
{
	if (CachedLoadingLevel && !CachedLoadingLevel->IsInViewport())
	{
		CachedLoadingLevel->AddToViewport(100);
	}
}

void ULCUIManager::HideLoadingLevel()
{
	if (CachedLoadingLevel)
	{
		if (CachedLoadingLevel->IsInViewport())
		{
			CachedLoadingLevel->RemoveFromParent();
		}

		/*if (CachedLoadingLevel->IsRooted())
		{
			CachedLoadingLevel->RemoveFromRoot();
		}*/

		// nullptr 처리 누락되면 GC 시 에러 발생
		CachedLoadingLevel = nullptr;
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
	SetInputModeUIOnly(CachedNotePopupWidget);
}

void ULCUIManager::HideNotePopup()
{
	if (CachedNotePopupWidget && CachedNotePopupWidget->IsInViewport())
	{
		CachedNotePopupWidget->RemoveFromParent();
	}
	ChangeHUD();
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
		LOG_Frame_WARNING(TEXT("SwitchToWidget : NewWidget == nullptr."));

		return;
	}

	if (CurrentWidget && CurrentWidget != NewWidget)
	{
		if (CurrentWidget->IsInViewport())
		{
			LOG_Frame_WARNING(TEXT("SwitchToWidget : CurrentWidget->RemoveFromParent."));

			CurrentWidget->RemoveFromParent();
		}
	}

	NewWidget->AddToViewport();
	NewWidget->SetVisibility(ESlateVisibility::Visible);

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

UTaskbarWidget* ULCUIManager::GetTaskbarWidget() const
{
	if (CachedDesktopWidget)
	{
		UTaskbarWidget* Taskbar = CachedDesktopWidget->GetTaskbarWidget();
		if (Taskbar)
		{
			LOG_Frame_WARNING(TEXT("GetTaskbarWidget() returned: %s"), *Taskbar->GetClass()->GetName());
			return Taskbar;
		}
	}
	LOG_Frame_WARNING(TEXT("GetTaskbarWidget() failed: Taskbar is nullptr"));
	return nullptr;
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

UShopWidget* ULCUIManager::ShowShopWidget(int32 Gold)
{
	if (!ShopWidgetClass)
	{
		LOG_Frame_WARNING(TEXT("ULCUIManager::ShowShopWidget - ShopWidgetClass is null."));
		return nullptr;
	}

	if (OwningPlayer == nullptr || OwningPlayer->IsLocalPlayerController() == false)
	{
		LOG_Frame_WARNING(TEXT("ULCUIManager::ShowShopWidget - Invalid OwningPlayer."));
		return nullptr;
	}

	// F 키 위젯 끄기
	if (LastShopInteractor && LastShopInteractor->GetDesktopWidgetComponent())
	{
		LastShopInteractor->GetDesktopWidgetComponent()->SetVisibility(false);
	}

	// 위젯이 이미 존재하면 다시 열기
	if (CachedShopWidget)
	{
		if (ULCDesktopWindowManager* WindowManager = GetDesktopWindowManager())
		{
			WindowManager->OpenWindow(CachedShopWidget);
		}
	}
	else
	{
		CachedShopWidget = CreateWidget<UShopWidget>(OwningPlayer, ShopWidgetClass);
		if (!CachedShopWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("ULCUIManager::ShowShopWidget - Failed to create ShopWidget."));
			return nullptr;
		}

		if (ULCDesktopWindowManager* WindowManager = GetDesktopWindowManager())
		{
			WindowManager->OpenWindow(CachedShopWidget);
		}

		if (UTaskbarWidget* Taskbar = GetTaskbarWidget())
		{
			Taskbar->AddAppButtonFor(CachedShopWidget);
		}
	}

	// HUD 감추고 창 전환
	HideHUD();
	// 골드 설정
	CachedShopWidget->SetGold(Gold);

	// 입력 차단 + UI 입력 모드
	if (APawn* Pawn = OwningPlayer->GetPawn())
	{
		Pawn->DisableInput(OwningPlayer);
	}
	SetInputModeUIOnly(CachedShopWidget);

	LOG_Frame_WARNING(TEXT("ULCUIManager::return CachedShopWidget."));

	return CachedShopWidget;
}

ULCDesktopWindowManager* ULCUIManager::GetDesktopWindowManager() const
{
	if (CachedDesktopWidget)
	{
		// DesktopWidget 내부에서 DesktopWindowManager를 직접 갖고 있어야 함
		return CachedDesktopWidget->GetWindowManager(); // 이 함수가 존재해야 함
	}

	UE_LOG(LogTemp, Warning, TEXT("ULCUIManager::GetDesktopWindowManager - CachedDesktopWidget is null."));
	return nullptr;
}
