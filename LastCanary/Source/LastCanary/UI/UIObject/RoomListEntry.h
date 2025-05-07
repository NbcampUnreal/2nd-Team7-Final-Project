#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/RoomInfo.h"
#include "RoomListEntry.generated.h"

/**
 * 
 */
class UTextBlock;
class UImage;
class UButton;
UCLASS()
class LASTCANARY_API URoomListEntry : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
private:
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* RoomID;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerCountText;
	UPROPERTY(meta = (BindWidget))
	UImage* PrivateIcon;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void OnJoinButtonClicked();

private:
	FRoomInfo RoomInfo;
};
