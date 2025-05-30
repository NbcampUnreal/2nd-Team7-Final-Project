#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LCUIManagerSettings.generated.h"

/**
 * 
 */
class UTitleMenu;
class ULobbyMenu;
class URoomListEntry;
class UEnterPasswordWidget;
class UOptionWidget;
class UInGameHUD;
class UShopWidget;
class UMapSelectWidget;
class UPopupCreateSession;
class UPopupLoading;
class UPopupNotice;
class UInventoryMainWidget;
class UBackpackInventoryWidget;
class UToolbarInventoryWidget;

UCLASS(BlueprintType)
class LASTCANARY_API ULCUIManagerSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UTitleMenu> FromBPTitleMenuClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<ULobbyMenu> FromBPLobbyMenuClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomListEntry> FromBPRoomListEntryClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEnterPasswordWidget> FromBPEnterPasswordWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UOptionWidget> FromBPOptionWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UInGameHUD> FromBPInGameHUDClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UShopWidget> FromBPShopWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UMapSelectWidget> FromBPMapSelectWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPopupCreateSession> FromBPCreateSessionWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPopupLoading> FromBPPopupLoadingClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPopupNotice> FromBPPopupNoticeClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UInventoryMainWidget> FromBPInventoryMainUIClass;
};
