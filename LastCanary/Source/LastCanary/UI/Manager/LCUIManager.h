#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Actor/ShopInteractor.h"
#include "Actor/MapSelectInteractor.h"
#include "LCUIManager.generated.h"

/**
 * 
 */
class ULCUIManagerSettings;
class UTitleMenu;
class ULobbyMenu;
class UEnterPasswordWidget;
class UOptionWidget;
class UInGameHUD;
class UShopWidget;
class UMapSelectWidget;
class UPopupCreateSession;
class UPopupLoading;
class UPopupNotice;
class ULoadingLevel;
class UInventoryMainWidget;
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
	void ShowEnterPasswordWidget(const FString& RoomID);
	void ShowInGameHUD();
	void HideInGameHUD();
	void ShowInventoryMainWidget();
	void HideInventoryMainWidget();
	void ShowOptionPopup();
	void ShowPauseMenu();
	void HidePauseMenu();
	bool IsPauseMenuOpen() const;
	void ShowConfirmPopup(TFunction<void()> OnConfirm);
	void ShowShopPopup();
	void HideShopPopup();
	void ShowMapSelectPopup();
	void HideMapSelectPopup();
	//void ShowCreateSession();
	void ToggleInventory();

	void SwitchToWidget(UUserWidget* Widget);

	/* 팝업 */
	void ShowCreateSession();
	UFUNCTION(BlueprintCallable)
	void ShowPopUpLoading();
	UFUNCTION(BlueprintCallable)
	void HidePopUpLoading();

	UFUNCTION(BlueprintCallable)
	void ShowPopupNotice(FString Notice);
	UFUNCTION(BlueprintCallable)
	void HidePopUpNotice();

	void ShowLoadingLevel();
	void HideLoadingLevel();

	/* 입력 모드 제어 */
	void SetInputModeUIOnly(UUserWidget* FocusWidget = nullptr);
	void SetInputModeGameOnly();

	/* 위젯 게터 */
	UTitleMenu* GetTitleMenu() const { return CachedTitleMenu; }
	ULobbyMenu* GetLobbyMenu() const { return CachedLobbyMenu; }
	UEnterPasswordWidget* GetEnterPasswordWidget() const { return CachedEnterPasswordWidget; }
	UOptionWidget* GetOptionWidget() const { return CachedOptionWidget; }
	UInGameHUD* GetInGameHUD() const { return CachedInGameHUD; }
	UInventoryMainWidget* GetInventoryMainWidget() const { return CachedInventoryMainWidget; }


	void SetLastShopInteractor(AShopInteractor* Interactor);
	void SetLastMapSelectInteractor(AMapSelectInteractor* Interactor);

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
	TSubclassOf<UEnterPasswordWidget> EnterPasswordWidgetClass;

	UPROPERTY()
	TSubclassOf<UOptionWidget> OptionWidgetClass;
	UPROPERTY()
	TSubclassOf<UOptionWidget> ConfirmPopupClass;
	UPROPERTY()
	TSubclassOf<UInGameHUD> InGameHUDWidgetClass;
	UPROPERTY()
	TSubclassOf<UShopWidget> ShopWidgetClass;
	UPROPERTY()
	TSubclassOf<UMapSelectWidget> MapSelectWidgetClass;

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

	// 위젯 캐싱
	UPROPERTY()
	UTitleMenu* CachedTitleMenu;
	UPROPERTY()
	ULobbyMenu* CachedLobbyMenu;
	UPROPERTY()
	UEnterPasswordWidget* CachedEnterPasswordWidget;
	UPROPERTY()
	UOptionWidget* CachedOptionWidget;
	UPROPERTY()
	UInGameHUD* CachedInGameHUD;
	UPROPERTY()
	UShopWidget* CachedShopWidget;
	UPROPERTY()
	UMapSelectWidget* CachedMapSelectWidget;

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
};
