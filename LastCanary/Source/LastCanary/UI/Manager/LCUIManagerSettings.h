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
class UOptionWidget;
class UPauseMenu;
class UInGameHUD;
class UShopWidget;
class UMapSelectWidget;
class UPopupCreateSession;
class UPopupLoading;
class UPopupNotice;
class ULoadingLevel;
class UInventoryMainWidget;
class UBackpackInventoryWidget;
class UToolbarInventoryWidget;
class UConfirmPopup;
class UChecklistWidget;
class UResultMenu;
class URoomWidget;
class UNotePopupWidget;
class UDroneHUD;

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
	TSubclassOf<UOptionWidget> FromBPOptionWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UPauseMenu> FromBPPauseMenuClass;
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
	TSubclassOf<ULoadingLevel> FromBPLoadingLevelClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UInventoryMainWidget> FromBPInventoryMainUIClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UConfirmPopup> FromBPConfirmPopupClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UChecklistWidget> FromBPChecklistWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UResultMenu> FromBPResultMenuClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomWidget> FromBPRoomWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UNotePopupWidget> FromBPNotePopupWidgetClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UDroneHUD> FromBPDroneHUDClass;
};
