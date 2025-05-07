#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "LobbyMenu.generated.h"

/**
 * 
 */
class UButton;
class UScrollBox;
UCLASS()
class LASTCANARY_API ULobbyMenu : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget))
	UButton* CreateRoomButton;
	UPROPERTY(meta = (BindWidget))
	UScrollBox* RoomListBox;

	UFUNCTION()
	void OnCreateRoomButtonClicked();
};
