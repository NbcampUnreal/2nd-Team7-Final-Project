#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "LCUIManager.generated.h"

/**
 * 
 */
class ULCUIManagerSettings;
class UTitleMenu;
class ULobbyMenu;
class UEnterPasswordWidget;
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
	void ShowEnterPasswordWidget(const FString& RoomID);

	void SwitchToWidget(UUserWidget* Widget);

	/* 위젯 게터 */
	UTitleMenu* GetTitleMenu() const { return CachedTitleMenu; }
	ULobbyMenu* GetLobbyMenu() const { return CachedLobbyMenu; }
	UEnterPasswordWidget* GetEnterPasswordWidget() const { return CachedEnterPasswordWidget; }

private:
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

	// 위젯 캐싱
	UPROPERTY()
	UTitleMenu* CachedTitleMenu;
	UPROPERTY()
	ULobbyMenu* CachedLobbyMenu;
	UPROPERTY()
	UEnterPasswordWidget* CachedEnterPasswordWidget;

};
