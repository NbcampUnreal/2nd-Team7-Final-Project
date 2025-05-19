#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Actor/ShopInteractor.h"
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

class UInventoryMainWidget;
UCLASS()
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
	void ShowShopPopup();
	// 인벤토리로 추가
	//void ShowInventoryPopup(); // 사용하지 않게 될 것 같음
	void ToggleInventoryPopup();
	// ---------------

	void HideShopPopup();

	void SwitchToWidget(UUserWidget* Widget);

	/* 위젯 게터 */
	UTitleMenu* GetTitleMenu() const { return CachedTitleMenu; }
	ULobbyMenu* GetLobbyMenu() const { return CachedLobbyMenu; }
	UEnterPasswordWidget* GetEnterPasswordWidget() const { return CachedEnterPasswordWidget; }

	void SetLastShopInteractor(AShopInteractor* Interactor);

private:
	UPROPERTY()
	AShopInteractor* LastShopInteractor;
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
	TSubclassOf<UInGameHUD> InGameHUDWidgetClass;
	UPROPERTY()
	TSubclassOf<UShopWidget> ShopWidgetClass;

	// 인벤토리로 추가
	UPROPERTY()
	TSubclassOf<UInventoryMainWidget> InventoryMainWidgetClass;
	// ---------------

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

	// 인벤토리로 추가
	UPROPERTY()
	UInventoryMainWidget* CachedInventoryMainWidget;
	// ---------------
};
