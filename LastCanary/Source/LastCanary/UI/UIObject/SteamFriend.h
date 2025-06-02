#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "SteamFriend.generated.h"

class UImage;
class UTextBlock;
class UButton;

UCLASS()
class LASTCANARY_API USteamFriend : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	/** ½æ³×ÀÏ ÀÌ¹ÌÁö À§Á¬ */
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite)
	UImage* AvatarImage;
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite)
	UTextBlock* NickNameText;
	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite)
	UButton* InviteButton;

	UFUNCTION()
	void OnInviteButtonClicked();
};
