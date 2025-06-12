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

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Session")
	void SetRoomListInfo();

	virtual void SetRoomListInfo_Implementation();

public:
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RoomID;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* RoomNameText;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UImage* LockIconImage;
	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	UTextBlock* PlayerCountText;
	//UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	//UTextBlock* PingText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> LockIcon;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSoftObjectPtr<UTexture2D> UnLockIcon;
	
private:
	//UPROPERTY(meta = (BindWidget))
	//UImage* PrivateIcon;
	UPROPERTY(meta = (BindWidget))
	UButton* JoinButton;

	UFUNCTION()
	void OnJoinButtonClicked();

private:
	FRoomInfo RoomInfo;
};
