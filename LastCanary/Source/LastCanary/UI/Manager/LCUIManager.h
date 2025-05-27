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
class UUIElementCreateSession;
class UPopupLoading;
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
	void ShowOptionPopup();
	void ShowPauseMenu();
	void HidePauseMenu();
	void ShowConfirmPopup(TFunction<void()> OnConfirm);
	void ShowShopPopup();
	void HideShopPopup();
	void ShowMapSelectPopup();
	void HideMapSelectPopup();
	void ShowCreateSession();

	void SwitchToWidget(UUserWidget* Widget);

	// TO DO : 로딩 팝업 표시
	UFUNCTION(BlueprintCallable)
	void ShowPopUpLoading();
	UFUNCTION(BlueprintCallable)
	void HidePopUpLoading();

	/* 입력 모드 제어 */
	void SetInputModeUIOnly(UUserWidget* FocusWidget = nullptr);
	void SetInputModeGameOnly();

	/* 위젯 게터 */
	UTitleMenu* GetTitleMenu() const { return CachedTitleMenu; }
	ULobbyMenu* GetLobbyMenu() const { return CachedLobbyMenu; }
	UEnterPasswordWidget* GetEnterPasswordWidget() const { return CachedEnterPasswordWidget; }
	UOptionWidget* GetOptionWidget() const { return CachedOptionWidget; }

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
	TSubclassOf<UUIElementCreateSession> CreateSessionClass;
	UPROPERTY()
	TSubclassOf<UPopupLoading> PopUpLoadingClass;

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

	UPROPERTY()
	UUIElementCreateSession* CachedCreateSession;
	UPROPERTY()
	UPopupLoading* CachedPopupLoading;
};
