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
#include "UI/UIElement/UIElementCreateSession.h"

#include "UI/UIObject/ConfirmPopup.h"
#include "UI/Popup/PopupLoading.h"

//#include "Framework/PlayerController/LCLobbyPlayerController.h"
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
	OwningPlayer = PlayerController;
	UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] OwningPlayer 설정: %s"),
		OwningPlayer ? *OwningPlayer->GetName() : TEXT("NULL"));
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
			PopUpLoadingClass = Settings->FromBPPopupLoadingClass;
			UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] InventoryMainWidgetClass: %s"),
				InventoryMainWidgetClass ? *InventoryMainWidgetClass->GetName() : TEXT("NULL"));
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
			//if ((CachedInventoryMainWidget == nullptr) && InventoryMainWidgetClass)
			//{
			//	CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(PlayerController, InventoryMainWidgetClass);
			//}
			if ((CachedCreateSession == nullptr) && CreateSessionClass)
			{
				CachedCreateSession = CreateWidget<UUIElementCreateSession>(PlayerController, CreateSessionClass);
			}
			if ((CachedPopupLoading == nullptr) && PopUpLoadingClass)
			{
				CachedPopupLoading = CreateWidget<UPopupLoading>(PlayerController, PopUpLoadingClass);
			}
			if ((CachedInventoryMainWidget == nullptr) && InventoryMainWidgetClass)
			{
				UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] 인벤토리 위젯 생성 시도"));
				CachedInventoryMainWidget = CreateWidget<UInventoryMainWidget>(PlayerController, InventoryMainWidgetClass);

				if (CachedInventoryMainWidget)
				{
					UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] 인벤토리 위젯 생성 성공: %s"),
						*CachedInventoryMainWidget->GetName());
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("[InitUIManager] 인벤토리 위젯 생성 실패!"));
				}
			}
			else
			{
				if (!InventoryMainWidgetClass)
				{
					UE_LOG(LogTemp, Error, TEXT("[InitUIManager] InventoryMainWidgetClass가 NULL"));
				}
				if (CachedInventoryMainWidget)
				{
					UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] 인벤토리 위젯이 이미 존재함"));
				}
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("[InitUIManager] 최종 CachedInventoryMainWidget: %s"),
		CachedInventoryMainWidget ? *CachedInventoryMainWidget->GetName() : TEXT("NULL"));
}

void ULCUIManager::SetPlayerController(APlayerController* PlayerController)
{
	OwningPlayer = PlayerController;
}

void ULCUIManager::ShowTitleMenu()
{
	LOG_Frame_WARNING(TEXT("ShowTitleMenu"));
	SwitchToWidget(CachedTitleMenu);
	SetInputModeUIOnly(CurrentWidget);
}

void ULCUIManager::ShowLobbyMenu()
{
	LOG_Frame_WARNING(TEXT("ShowLobbyMenu"));
	SwitchToWidget(CachedLobbyMenu);
	SetInputModeUIOnly(CurrentWidget);
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
	SetInputModeUIOnly(CurrentWidget);
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

	SetInputModeUIOnly(CurrentWidget);
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
	HideInGameHUD();

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
	}
	SetInputModeUIOnly(CurrentWidget);
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
	}
	SetInputModeGameOnly();
	ShowInGameHUD();
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

void ULCUIManager::ToggleInventory()
{
	// ⭐ 더 이상 InitializeInventorySystem() 호출하지 않음
	if (!CachedInventoryMainWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("[ToggleInventory] 인벤토리 위젯이 없습니다. InitUIManager가 제대로 호출되지 않았을 수 있습니다."));
		return;
	}

	// 뷰포트에 없다면 추가 (첫 호출 시)
	if (!CachedInventoryMainWidget->IsInViewport())
	{
		CachedInventoryMainWidget->AddToViewport(1);
		CachedInventoryMainWidget->ShowToolbarOnly(); // 툴바는 항상 표시
	}

	// 가방 토글
	CachedInventoryMainWidget->ToggleBackpackInventory();

	// 입력 모드 변경
	if (OwningPlayer)
	{
		if (CachedInventoryMainWidget->IsBackpackInventoryOpen())
		{
			// 가방 열림
			FInputModeGameAndUI InputMode;
			InputMode.SetWidgetToFocus(CachedInventoryMainWidget->TakeWidget());
			InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			OwningPlayer->SetInputMode(InputMode);
			OwningPlayer->bShowMouseCursor = true;
		}
		else
		{
			// 가방 닫힘
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
	LOG_Frame_WARNING(TEXT("ShowInGameHUD"));
	SwitchToWidget(CachedInGameHUD);
	SetInputModeGameOnly();

	UE_LOG(LogTemp, Warning, TEXT("[ShowInGameHUD] CachedInventoryMainWidget 상태: %s"),
		CachedInventoryMainWidget ? *CachedInventoryMainWidget->GetName() : TEXT("NULL"));

	if (CachedInventoryMainWidget)
	{
		if (!CachedInventoryMainWidget->IsInViewport())
		{
			UE_LOG(LogTemp, Warning, TEXT("[ShowInGameHUD] 인벤토리 위젯을 뷰포트에 추가"));
			CachedInventoryMainWidget->AddToViewport(1);
		}
		CachedInventoryMainWidget->ShowToolbarOnly();
		UE_LOG(LogTemp, Warning, TEXT("[ShowInGameHUD] 툴바 표시 완료"));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[ShowInGameHUD] 인벤토리 위젯이 없음. InitUIManager가 아직 호출되지 않았을 수 있습니다."));
	}
}

void ULCUIManager::HideInGameHUD()
{
	LOG_Frame_WARNING(TEXT("HideInGameHUD"));
	if (CachedInGameHUD && CachedInGameHUD->IsInViewport())
	{
		CachedInGameHUD->RemoveFromParent();
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
