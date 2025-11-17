#include "UI/UIObject/SteamFriend.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

void USteamFriend::NativeConstruct()
{
	Super::NativeConstruct();

	if (InviteButton)
	{
		InviteButton->OnClicked.AddUniqueDynamic(this, &USteamFriend::OnInviteButtonClicked);
	}
}

void USteamFriend::NativeDestruct()
{
	Super::NativeDestruct();

	if (InviteButton)
	{
		InviteButton->OnClicked.RemoveDynamic(this, &USteamFriend::OnInviteButtonClicked);
	}

}

void USteamFriend::OnInviteButtonClicked()
{

}
