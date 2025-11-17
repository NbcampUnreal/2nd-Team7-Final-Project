#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PopupSteamInvite.generated.h"

class UScrollBox;
class UVerticalBox;
class USteamFriend;

UCLASS()
class LASTCANARY_API UPopupSteamInvite : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget), EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UVerticalBox> FriendContainer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<USteamFriend> SteamFriendClass;
};
