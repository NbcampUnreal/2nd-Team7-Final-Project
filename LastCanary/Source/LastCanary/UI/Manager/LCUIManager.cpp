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
			InventoryMainWidgetClass = Settings->FromBPInventoryMainUIClass;
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

	if (!IsValid(CachedOptionWidget))
	{
		CachedOptionWidget = CreateWidget<UOptionWidget>(GetWorld(), OptionWidgetClass);
	}

	if (CachedOptionWidget && CachedOptionWidget->IsInViewport() == false)
	{
		CachedOptionWidget->AddToViewport(1);
	}

	if (CachedOptionWidget)
	{
		SetInputModeUIOnly(CachedOptionWidget);
	}
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
	UE_LOG(LogTemp, Warning, TEXT("=== [ULCUIManager::ToggleInventoryPopup] 함수 진입 ==="));

	// 기본 상태 확인
	UE_LOG(LogTemp, Warning, TEXT("OwningPlayer: %s"), OwningPlayer ? *OwningPlayer->GetName() : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("InventoryMainWidgetClass: %s"), InventoryMainWidgetClass ? TEXT("Valid") : TEXT("NULL"));
	UE_LOG(LogTemp, Warning, TEXT("CachedInventoryMainWidget: %s"), CachedInventoryMainWidget ? TEXT("Valid") : TEXT("NULL"));

	if (!CachedInventoryMainWidget && InventoryMainWidgetClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 위젯 생성 시작"));

		CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(OwningPlayer, InventoryMainWidgetClass);

		if (CachedInventoryMainWidget)
		{
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 위젯 생성 성공: %s"),
				*CachedInventoryMainWidget->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 위젯 생성 실패"));
			UE_LOG(LogTemp, Warning, TEXT("=== [ULCUIManager::ToggleInventoryPopup] 종료 (위젯 생성 실패) ==="));
			return;
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 위젯 생성 스킵 - 이미 존재하거나 클래스 없음"));
	}

	if (CachedInventoryMainWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 위젯 상태 확인"));

		bool bIsInViewport = CachedInventoryMainWidget->IsInViewport();
		ESlateVisibility CurrentVisibility = CachedInventoryMainWidget->GetVisibility();
		bool bIsCurrentlyVisible = bIsInViewport && (CurrentVisibility == ESlateVisibility::Visible);

		UE_LOG(LogTemp, Warning, TEXT("IsInViewport: %s"), bIsInViewport ? TEXT("True") : TEXT("False"));
		UE_LOG(LogTemp, Warning, TEXT("Visibility: %d (%s)"), (int32)CurrentVisibility,
			CurrentVisibility == ESlateVisibility::Visible ? TEXT("Visible") :
			CurrentVisibility == ESlateVisibility::Collapsed ? TEXT("Collapsed") : TEXT("Other"));
		UE_LOG(LogTemp, Warning, TEXT("IsCurrentlyVisible: %s"), bIsCurrentlyVisible ? TEXT("True") : TEXT("False"));

		if (bIsCurrentlyVisible)
		{
			// 인벤토리 숨기기
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 숨기기 시작"));

			CachedInventoryMainWidget->SetVisibility(ESlateVisibility::Collapsed);
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] SetVisibility(Collapsed) 완료"));

			if (OwningPlayer)
			{
				OwningPlayer->SetInputMode(FInputModeGameOnly());
				OwningPlayer->bShowMouseCursor = false;
				UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 입력 모드를 GameOnly로 변경"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] OwningPlayer가 NULL"));
			}

			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 숨기기 완료"));
		}
		else
		{
			// 인벤토리 표시
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 표시 시작"));

			if (!bIsInViewport)
			{
				UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 뷰포트에 위젯 추가"));
				CachedInventoryMainWidget->AddToViewport();
				UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] AddToViewport() 완료"));
			}

			CachedInventoryMainWidget->SetVisibility(ESlateVisibility::Visible);
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] SetVisibility(Visible) 완료"));

			// RefreshInventory 호출 확인
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] RefreshInventory() 호출 시작"));
			CachedInventoryMainWidget->RefreshInventory();
			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] RefreshInventory() 호출 완료"));

			if (OwningPlayer)
			{
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(CachedInventoryMainWidget->TakeWidget());
				InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
				OwningPlayer->SetInputMode(InputMode);
				OwningPlayer->bShowMouseCursor = true;
				UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 입력 모드를 GameAndUI로 변경"));
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] OwningPlayer가 NULL"));
			}

			UE_LOG(LogTemp, Warning, TEXT("[ULCUIManager::ToggleInventoryPopup] 인벤토리 표시 완료"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ULCUIManager::ToggleInventoryPopup] CachedInventoryMainWidget이 여전히 NULL"));

		// 위젯 클래스 상태 재확인
		if (!InventoryMainWidgetClass)
		{
			UE_LOG(LogTemp, Error, TEXT("[ULCUIManager::ToggleInventoryPopup] InventoryMainWidgetClass가 설정되지 않음"));
		}
		if (!OwningPlayer)
		{
			UE_LOG(LogTemp, Error, TEXT("[ULCUIManager::ToggleInventoryPopup] OwningPlayer가 NULL"));
		}
	}

	UE_LOG(LogTemp, Warning, TEXT("=== [ULCUIManager::ToggleInventoryPopup] 함수 종료 ==="));
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
