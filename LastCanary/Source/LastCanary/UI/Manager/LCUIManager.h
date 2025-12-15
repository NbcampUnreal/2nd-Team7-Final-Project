#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/GameResultData.h"
#include "Actor/ShopInteractor.h"
#include "LCUIManager.generated.h"

// =========================================================
// Forward Declarations (실제로 멤버/시그니처에 쓰이는 것만)
// =========================================================
class ULCUIManagerSettings;

class UTitleMenu;
class ULobbyMenu;
class UOptionWidget;
class UPauseMenu;
class UInGameHUD;
class UShopWidget;
class UPopupCreateSession;
class UPopupLoading;
class UPopupNotice;
class ULoadingLevel;
class UInventoryMainWidget;
class UChecklistWidget;
class UResultMenu;
class UResultWidget;
class URoomWidget;
class UNotePopupWidget;
class UDroneHUD;
class USpectatorWidget;
class UGameOverWidget;
class UGameEndWidget;
class UServerMessageWidget;
class UDesktopWidget;
class UTaskbarWidget;
class ULCDesktopWindowManager;
class UCharacterCustomizationWidget;
class UConfirmPopup;
class USelectionWheelWidget;

// =========================================================
// Enum
// =========================================================
UENUM(BlueprintType)
enum class ELCUIContext : uint8
{
	None,
	Title,
	Room,
	InGame,
	Spectator,
	DroneHUD
};

// =========================================================
// ULCUIManager
// =========================================================
UCLASS(BlueprintType)
class LASTCANARY_API ULCUIManager : public UObject
{
	GENERATED_BODY()

public:
	// =====================================================
	// Constructor & Init
	// =====================================================
	ULCUIManager();
	void InitUIManager(APlayerController* PlayerController);
	void SetPlayerController(APlayerController* PlayerController);

	// =====================================================
	// High-level UI Flow (Show / Hide / Toggle)
	// =====================================================
	void ShowTitleMenu();
	void ShowLobbyMenu();
	void ChangeHUD();
	void HideHUD();

	void ShowInventoryMainWidget();
	void HideInventoryMainWidget();
	void ToggleInventory();

	void ShowOptionWidget();
	void HideOptionWidget();

	void ShowPauseMenu();
	void HidePauseMenu();
	bool IsPauseMenuOpen() const;

	void ShowChecklistWidget();
	void ShowNewChecklistWidget(UDataTable* CheckListTable);

	UResultMenu* ShowResultMenu();
	UResultWidget* ShowResultWidget();

	void ShowRoomWidget();
	void HideRoomWidget();

	void ShowGameOverWidget();
	void HideGameOverWidget();

	void ShowGameEndWidget();
	void HideGameEndWidget();
	void ToggleGameEndWidget();

	void ShowDesktop();
	void HideDesktop();

	UCharacterCustomizationWidget* ShowCharacterCustomizationWidget();

	// =====================================================
	// Special Popups / Notices
	// =====================================================
	void ShowConfirmPopup(TFunction<void()> OnConfirm, const FText& Message);

	void ShowCreateSession();

	UFUNCTION(BlueprintCallable)
	void ShowPopUpLoading();
	UFUNCTION(BlueprintCallable)
	void HidePopUpLoading();

	UFUNCTION(BlueprintCallable)
	void ShowPopupNotice(const FText& Notice);
	UFUNCTION(BlueprintCallable)
	void HidePopUpNotice();

	void ShowLoadingLevel();
	void RestoreLoadingScreenIfNeeded();
	void HideLoadingLevel();

	// =====================================================
	// Note Popup
	// =====================================================
	UFUNCTION(BlueprintCallable, Category = "UI|Note")
	void ShowNotePopup(const FText& NoteText, const TArray<TSoftObjectPtr<UTexture2D>>& CandidateImages, int32 SelectedIndex);
	UFUNCTION(BlueprintCallable, Category = "UI|Note")
	void HideNotePopup();

	// =====================================================
	// Server Message
	// =====================================================
	UFUNCTION(BlueprintCallable)
	void AddServerMessage(const FString& Message);

	// =====================================================
	// Input Mode / Context
	// =====================================================
	void SetInputModeUIOnly(UUserWidget* FocusWidget = nullptr);
	void SetInputModeGameOnly();
	void SetInputModeGameAndUI();
	void UpdateInputModeByContext();

	void SetUIContext(ELCUIContext NewContext);
	const ELCUIContext GetUIContext() const;

	// =====================================================
	// Shop
	// =====================================================
	UFUNCTION(BlueprintCallable)
	UShopWidget* ShowShopWidget(int32 Gold);

	void HideShopPopup();
	void SetLastShopInteractor(AShopInteractor* Interactor);

	TSubclassOf<UShopWidget> GetShopWidgetClass() const { return ShopWidgetClass; }
	ULCDesktopWindowManager* GetDesktopWindowManager() const;

	// =====================================================
	// Selection Wheel
	// =====================================================
	UPROPERTY(EditAnywhere, Category = "UI|Popup")
	TSubclassOf<USelectionWheelWidget> SelectionWheelClass;

	UPROPERTY()
	USelectionWheelWidget* CachedSelectionWheel = nullptr;

	UFUNCTION()
	void ShowSelectionWheel();
	UFUNCTION()
	void HideSelectionWheel();

	UFUNCTION(BlueprintCallable)
	USelectionWheelWidget* GetSelectionWheel() const { return CachedSelectionWheel; }

	// =====================================================
	// Getters (Cached Widgets)
	// =====================================================
	UTitleMenu* GetTitleMenu() const { return CachedTitleMenu; }
	ULobbyMenu* GetLobbyMenu() const { return CachedLobbyMenu; }
	UOptionWidget* GetOptionWidget() const { return CachedOptionWidget; }
	UPauseMenu* GetPauseMenu() const { return CachedPauseMenu; }
	UInGameHUD* GetInGameHUD() const { return CachedInGameHUD; }
	UShopWidget* GetCachedShopWidget() const { return CachedShopWidget; }
	UInventoryMainWidget* GetInventoryMainWidget() const { return CachedInventoryMainWidget; }
	UChecklistWidget* GetChecklistWidget() const { return CachedChecklistWidget; }
	UResultMenu* GetResultMenu() const { return CachedResultMenu; }
	UResultWidget* GetResultWidget() const { return CachedResultWidget; }
	URoomWidget* GetRoomWidget() const { return CachedRoomWidget; }
	UDroneHUD* GetDroneHUD() const { return CachedDroneHUD; }
	USpectatorWidget* GetSpectatorWidget() const { return CachedSpectatorWidget; }
	UGameOverWidget* GetGameOverWidget() const { return CachedGameOverWidget; }
	UGameEndWidget* GetGameEndWidget() const { return CachedGameEndWidget; }
	UServerMessageWidget* GetServerMessageWidget() const { return CachedServerMessageWidget; }
	UDesktopWidget* GetDesktopWidget() const { return CachedDesktopWidget; }
	UCharacterCustomizationWidget* GetCharacterCustomizationWidget() const { return CachedCharacterCustomizationWidget; }

	UTaskbarWidget* GetTaskbarWidget() const;

	// =====================================================
	// Error Handling
	// =====================================================
	UFUNCTION(BlueprintCallable)
	void SetSessionErrorState(const FText& Reason);

	UFUNCTION(BlueprintCallable)
	UInGameHUD* GetInGameHUDBlueprint() const { return CachedInGameHUD; }
	UFUNCTION(BlueprintCallable)
	USpectatorWidget* GetSpectatorWidgetBlueprint() const { return CachedSpectatorWidget; }


private:
	// =====================================================
	// Internal helper
	// =====================================================
	template<typename T>
	T* CreateAndCacheWidget(T*& CachedWidget, TSubclassOf<UUserWidget> WidgetClass)
	{
		if (CachedWidget == nullptr && WidgetClass)
		{
			CachedWidget = CreateWidget<T>(OwningPlayer, WidgetClass);
		}
		return CachedWidget;
	}

	void SwitchToWidget(UUserWidget* Widget);

	// =====================================================
	// References & Context
	// =====================================================
	UPROPERTY()
	APlayerController* OwningPlayer = nullptr;

	UPROPERTY()
	ULCUIManagerSettings* UIManagerSettings = nullptr;

	UPROPERTY()
	UUserWidget* CurrentWidget = nullptr;

	UPROPERTY()
	ELCUIContext CurrentContext = ELCUIContext::None;

	UPROPERTY()
	AShopInteractor* LastShopInteractor = nullptr;

public:
	// (원래 public에 있던 멤버라 그대로 둠)
	UPROPERTY()
	ULCDesktopWindowManager* DesktopWindowManager = nullptr;

private:
	// =====================================================
	// Widget Classes (BP settings에서 주입)
	// =====================================================
	UPROPERTY() TSubclassOf<UTitleMenu> TitleMenuClass;
	UPROPERTY() TSubclassOf<ULobbyMenu> LobbyMenuClass;
	UPROPERTY() TSubclassOf<UOptionWidget> OptionWidgetClass;
	UPROPERTY() TSubclassOf<UConfirmPopup> ConfirmPopupClass;
	UPROPERTY() TSubclassOf<UInGameHUD> InGameHUDClass;
	UPROPERTY() TSubclassOf<UShopWidget> ShopWidgetClass;
	UPROPERTY() TSubclassOf<UPauseMenu> PauseMenuClass;
	UPROPERTY() TSubclassOf<UPopupCreateSession> CreateSessionClass;
	UPROPERTY() TSubclassOf<UPopupLoading> PopupLoadingClass;
	UPROPERTY() TSubclassOf<UPopupNotice> PopupNoticeClass;
	UPROPERTY() TSubclassOf<ULoadingLevel> LoadingLevelClass;
	UPROPERTY() TSubclassOf<UInventoryMainWidget> InventoryMainWidgetClass;
	UPROPERTY() TSubclassOf<UChecklistWidget> ChecklistWidgetClass;
	UPROPERTY() TSubclassOf<UResultMenu> ResultMenuClass;
	UPROPERTY() TSubclassOf<UResultWidget> ResultWidgetClass;
	UPROPERTY() TSubclassOf<URoomWidget> RoomWidgetClass;
	UPROPERTY() TSubclassOf<UUserWidget> NotePopupWidgetClass;
	UPROPERTY() TSubclassOf<UDroneHUD> DroneHUDClass;
	UPROPERTY() TSubclassOf<USpectatorWidget> SpectatorWidgetClass;
	UPROPERTY() TSubclassOf<UGameOverWidget> GameOverWidgetClass;
	UPROPERTY() TSubclassOf<UGameEndWidget> GameEndWidgetClass;
	UPROPERTY() TSubclassOf<UServerMessageWidget> ServerMessageWidgetClass;
	UPROPERTY() TSubclassOf<UDesktopWidget> DesktopWidgetClass;
	UPROPERTY() TSubclassOf<UCharacterCustomizationWidget> CharacterCustomizationWidgetClass;

	// =====================================================
	// Widget Instances (캐시)
	// =====================================================
	UPROPERTY() UTitleMenu* CachedTitleMenu = nullptr;
	UPROPERTY() ULobbyMenu* CachedLobbyMenu = nullptr;
	UPROPERTY() UOptionWidget* CachedOptionWidget = nullptr;
	UPROPERTY() UInGameHUD* CachedInGameHUD = nullptr;
	UPROPERTY() UShopWidget* CachedShopWidget = nullptr;
	UPROPERTY() UPauseMenu* CachedPauseMenu = nullptr;
	UPROPERTY() UPopupLoading* CachedPopupLoading = nullptr;
	UPROPERTY() UPopupNotice* CachedPopupNotice = nullptr;
	UPROPERTY() ULoadingLevel* CachedLoadingLevel = nullptr;
	UPROPERTY() UInventoryMainWidget* CachedInventoryMainWidget = nullptr;
	UPROPERTY() UChecklistWidget* CachedChecklistWidget = nullptr;
	UPROPERTY() UResultMenu* CachedResultMenu = nullptr;
	UPROPERTY() UResultWidget* CachedResultWidget = nullptr;
	UPROPERTY() URoomWidget* CachedRoomWidget = nullptr;
	UPROPERTY() UNotePopupWidget* CachedNotePopupWidget = nullptr;
	UPROPERTY() UDroneHUD* CachedDroneHUD = nullptr;
	UPROPERTY() USpectatorWidget* CachedSpectatorWidget = nullptr;
	UPROPERTY() UGameOverWidget* CachedGameOverWidget = nullptr;
	UPROPERTY() UGameEndWidget* CachedGameEndWidget = nullptr;
	UPROPERTY() UServerMessageWidget* CachedServerMessageWidget = nullptr;
	UPROPERTY() UDesktopWidget* CachedDesktopWidget = nullptr;
	UPROPERTY() UCharacterCustomizationWidget* CachedCharacterCustomizationWidget = nullptr;

	// =====================================================
	// Session Error Info
	// =====================================================
	bool bSessionErrorOccurred = false;
	FText CachedErrorReason;
};
