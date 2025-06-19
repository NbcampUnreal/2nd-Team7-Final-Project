#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/GameResultData.h"
#include "Actor/ShopInteractor.h"
#include "LCUIManager.generated.h"

//-----------------
// Forward Declarations
//-------------------
class ULCUIManagerSettings;
class UTitleMenu;
class ULobbyMenu;
class UOptionWidget;
class UInGameHUD;
class UShopWidget;
class UPopupCreateSession;
class UPopupLoading;
class UPopupNotice;
class ULoadingLevel;
class UInventoryMainWidget;
class UPauseMenu;
class UConfirmPopup;
class UChecklistWidget;
class UResultMenu;
class URoomWidget;
class UNotePopupWidget;
class UDroneHUD;
class USpectatorWidget;
class UGameOverWidget;

//-----------------
// Enum
//-------------------
UENUM(BlueprintType)
enum class ELCUIContext : uint8
{
	None,
	Title,
	Room,
	InGame,
};

//-----------------
// ULCUIManager Class
//-------------------
UCLASS(BlueprintType)
class LASTCANARY_API ULCUIManager : public UObject
{
	GENERATED_BODY()

public:
	//-----------------
	// Constructor & Init
	//-------------------
	ULCUIManager();
	void InitUIManager(APlayerController* PlayerController);
	void SetPlayerController(APlayerController* PlayerController);

	//-----------------
	// UI Show/Hide Logic
	//-------------------
	void ShowTitleMenu();
	void ShowLobbyMenu();
	void ShowRoomListMenu();
	void ShowInGameHUD();
	void HideInGameHUD();
	void ShowInventoryMainWidget();
	void HideInventoryMainWidget();
	void ShowOptionWidget();
	void HideOptionWidget();
	void ShowPauseMenu();
	void HidePauseMenu();
	bool IsPauseMenuOpen() const;
	void ShowConfirmPopup(TFunction<void()> OnConfirm, const FText& Message);
	void ShowShopPopup(int Gold);
	void HideShopPopup();
	//void ShowCreateSession();
	void ToggleInventory();
	void ShowChecklistWidget();
	UResultMenu* ShowResultMenu();
	void ShowRoomWidget();
	void HideRoomWidget();
	void ShowDroneHUD();
	void HideDroneHUD();
	void ShowSpectatorWidget();
	void HideSpectatorWidget();
	void ShowGameOverWidget();
	void HideGameOverWidget();

	//-----------------
	// Special Popups
	//-------------------
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
	void HideLoadingLevel();

	//-----------------
	// Note Popup
	//-------------------
	UFUNCTION(BlueprintCallable, Category = "UI|Note")
	void ShowNotePopup(const FText& NoteText, const TArray<TSoftObjectPtr<UTexture2D>>& CandidateImages, int32 SelectedIndex);
	UFUNCTION(BlueprintCallable, Category = "UI|Note")
	void HideNotePopup();

	//-----------------
	// Input Mode Handling
	//-------------------
	void SetInputModeUIOnly(UUserWidget* FocusWidget = nullptr);
	void SetInputModeGameOnly();
	void SetInputModeGameAndUI();
	void UpdateInputModeByContext();
	void SetUIContext(ELCUIContext NewContext);
	const ELCUIContext GetUIContext() const;

	//-----------------
	// Widget Switching
	//-------------------
	void SwitchToWidget(UUserWidget* Widget);

	//-----------------
	// Getter Functions
	//-------------------
	FORCEINLINE UTitleMenu* GetTitleMenu() const { return CachedTitleMenu; }
	FORCEINLINE ULobbyMenu* GetLobbyMenu() const { return CachedLobbyMenu; }
	FORCEINLINE UOptionWidget* GetOptionWidget() const { return CachedOptionWidget; }
	FORCEINLINE UPauseMenu* GetPauseMenu() const { return CachedPauseMenu; }
	FORCEINLINE UInGameHUD* GetInGameHUD() const { return CachedInGameHUD; }
	FORCEINLINE UInventoryMainWidget* GetInventoryMainWidget() const { return CachedInventoryMainWidget; }
	FORCEINLINE UChecklistWidget* GetChecklistWidget() const { return CachedChecklistWidget; }
	FORCEINLINE UResultMenu* GetResultMenuClass() const { return CachedResultMenu; }
	FORCEINLINE URoomWidget* GetRoomWidgetInstance() const { return CachedRoomWidget; }
	FORCEINLINE UDroneHUD* GetDroneHUD() const { return CachedDroneHUD; }
	FORCEINLINE USpectatorWidget* GetSpectatorWidget() const { return CachedSpectatorWidget; }
	FORCEINLINE UGameOverWidget* GetGameOverWidget() const { return CachedGameOverWidget; }

	//-----------------
	// External Interactor Tracking
	//-------------------
	void SetLastShopInteractor(AShopInteractor* Interactor);

	//-----------------
	// Error Handling
	//-------------------
	UFUNCTION(BlueprintCallable)
	void SetSessionErrorState(const FText& Reason);

private:
	//-----------------
	// References & Context
	//-------------------
	UPROPERTY()
	APlayerController* OwningPlayer;
	UPROPERTY()
	ULCUIManagerSettings* UIManagerSettings;
	UPROPERTY()
	UUserWidget* CurrentWidget;
	UPROPERTY()
	ELCUIContext CurrentContext;

	UPROPERTY()
	AShopInteractor* LastShopInteractor;

	//-----------------
	// Widget Classes
	//-------------------
	UPROPERTY()
	TSubclassOf<UTitleMenu> TitleMenuClass;
	UPROPERTY()
	TSubclassOf<ULobbyMenu> LobbyMenuClass;
	UPROPERTY()
	TSubclassOf<UOptionWidget> OptionWidgetClass;
	UPROPERTY()
	TSubclassOf<UConfirmPopup> ConfirmPopupClass;
	UPROPERTY()
	TSubclassOf<UInGameHUD> InGameHUDWidgetClass;
	UPROPERTY()
	TSubclassOf<UShopWidget> ShopWidgetClass;
	UPROPERTY()
	TSubclassOf<UPauseMenu> PauseMenuClass;
	UPROPERTY()
	TSubclassOf<UPopupCreateSession> CreateSessionClass;
	UPROPERTY()
	TSubclassOf<UPopupLoading> PopUpLoadingClass;
	UPROPERTY()
	TSubclassOf<UPopupNotice> PopUpNoticeClass;
	UPROPERTY()
	TSubclassOf<ULoadingLevel> LoadingLevelClass;
	UPROPERTY()
	TSubclassOf<UInventoryMainWidget> InventoryMainWidgetClass;
	UPROPERTY()
	TSubclassOf<UChecklistWidget> ChecklistWidgetClass;
	UPROPERTY()
	TSubclassOf<UResultMenu> ResultMenuClass;
	UPROPERTY()
	TSubclassOf<URoomWidget> RoomWidgetClass;
	UPROPERTY()
	TSubclassOf<UUserWidget> NotePopupWidgetClass;
	UPROPERTY()
	TSubclassOf<UDroneHUD> DroneHUDClass;
	UPROPERTY()
	TSubclassOf<USpectatorWidget> SpectatorWidgetClass;
	UPROPERTY()
	TSubclassOf<UGameOverWidget> GameOverWidgetClass;

	//-----------------
	// Widget Instances
	//-------------------
	UPROPERTY()
	UTitleMenu* CachedTitleMenu;
	UPROPERTY()
	ULobbyMenu* CachedLobbyMenu;
	UPROPERTY()
	UOptionWidget* CachedOptionWidget;
	UPROPERTY()
	UInGameHUD* CachedInGameHUD;
	UPROPERTY()
	UShopWidget* CachedShopWidget;
	UPROPERTY()
	UPauseMenu* CachedPauseMenu;
	//UPROPERTY()
	//UPopupCreateSession* CachedCreateSession;
	UPROPERTY()
	UPopupLoading* CachedPopupLoading;
	UPROPERTY()
	UPopupNotice* CachedPopupNotice;
	UPROPERTY()
	ULoadingLevel* CachedLoadingLevel;
	UPROPERTY()
	UInventoryMainWidget* CachedInventoryMainWidget;
	UPROPERTY()
	UChecklistWidget* CachedChecklistWidget;
	UPROPERTY()
	UResultMenu* CachedResultMenu;
	UPROPERTY()
	URoomWidget* CachedRoomWidget;
	UPROPERTY()
	UNotePopupWidget* CachedNotePopupWidget;
	UPROPERTY()
	UDroneHUD* CachedDroneHUD;
	UPROPERTY()
	USpectatorWidget* CachedSpectatorWidget;
	UPROPERTY()
	UGameOverWidget* CachedGameOverWidget;

	//-----------------
	// Session Error Info
	//-------------------
	bool bSessionErrorOccurred = false;
	FText CachedErrorReson;
};
