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
UCLASS(BlueprintType)
class LASTCANARY_API ULCUIManagerSettings : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UTitleMenu> TitleMenuClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<ULobbyMenu> LobbyMenuClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<URoomListEntry> RoomListEntryClass;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<UEnterPasswordWidget> EnterPasswordWidgetClass;
};
