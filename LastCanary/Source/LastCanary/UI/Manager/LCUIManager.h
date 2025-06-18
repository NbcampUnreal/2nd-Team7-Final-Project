#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/GameResultData.h"
#include "Actor/ShopInteractor.h"
#include "Actor/MapSelectInteractor.h"
#include "LCUIManager.generated.h"

/**
 * 
 */
class ULCUIManagerSettings;
class UTitleMenu;
class ULobbyMenu;
class UOptionWidget;
class UInGameHUD;
class UShopWidget;
class UMapSelectWidget;
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

UENUM(BlueprintType)
enum class ELCUIContext : uint8
{
	None,
	Title,
	Room,
	InGame,
};

UCLASS(BlueprintType)
class LASTCANARY_API ULCUIManager : public UObject
{
	GENERATED_BODY()

public:
	ULCUIManager();
	void InitUIManager(APlayerController* PlayerController);
	void SetPlayerController(APlayerController* PlayerController);

	/* 메뉴 변경 */
	void ShowTitleMenu();
	void ShowLobbyMenu();
	void ShowRoomListMenu();
	void ShowInGameHUD();
	void HideInGameHUD();
	void ShowInventoryMainWidget();
	void HideInventoryMainWidget();
	void ShowOptionPopup();
	void ShowPauseMenu();
	void HidePauseMenu();
	bool IsPauseMenuOpen() const;
	void ShowConfirmPopup(TFunction<void()> OnConfirm, const FText& Message);
	void ShowShopPopup(int Gold);
	void HideShopPopup();
	void ShowMapSelectPopup();
	void HideMapSelectPopup();
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

	void SwitchToWidget(UUserWidget* Widget);

	/* 팝업 */
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

	UFUNCTION(BlueprintCallable, Category = "UI|Note")
	void ShowNotePopup(const FText& NoteText, const TArray<TSoftObjectPtr<UTexture2D>>& CandidateImages, int32 SelectedIndex);
	UFUNCTION(BlueprintCallable, Category = "UI|Note")
	void HideNotePopup();

	/* 입력 모드 제어 */
	void SetInputModeUIOnly(UUserWidget* FocusWidget = nullptr);
	void SetInputModeGameOnly();
	void SetInputModeGameAndUI();

	/* 위젯 게터 */
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

	void SetLastShopInteractor(AShopInteractor* Interactor);
	void SetLastMapSelectInteractor(AMapSelectInteractor* Interactor);

	void UpdateInputModeByContext();
	void SetUIContext(ELCUIContext NewContext);

private:
	UPROPERTY()
	AShopInteractor* LastShopInteractor;
	UPROPERTY()
	AMapSelectInteractor* LastMapSelectInteractor;
	UPROPERTY()
	APlayerController* OwningPlayer;
	UPROPERTY()
	ULCUIManagerSettings* UIManagerSettings;
	UPROPERTY()
	UUserWidget* CurrentWidget;

	/* 메뉴 클래스 */
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
	TSubclassOf<UMapSelectWidget> MapSelectWidgetClass;
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

	// 위젯 캐싱
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
	UMapSelectWidget* CachedMapSelectWidget;
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
	ELCUIContext CurrentContext;

	bool bSessionErrorOccurred = false;
	FText CachedErrorReson;
public:
	UFUNCTION(BlueprintCallable)
	void SetSessionErrorState(const FText& Reason);
};
