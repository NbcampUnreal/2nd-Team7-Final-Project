#include "UI/UIObject/RoomListEntry.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

#include "Framework/PlayerController/LCLobbyPlayerController.h"

void URoomListEntry::NativeConstruct()
{
	Super::NativeConstruct();
	if (JoinButton)
	{
		JoinButton->OnClicked.AddUniqueDynamic(this, &URoomListEntry::OnJoinButtonClicked);
    }
}

void URoomListEntry::NativeDestruct()
{
    Super::NativeDestruct();
    if (JoinButton)
    {
        JoinButton->OnClicked.RemoveDynamic(this, &URoomListEntry::OnJoinButtonClicked);
	}
}

void URoomListEntry::OnJoinButtonClicked()
{
    if (RoomInfo.bIsPrivate)
    {
        /*if (UEnterPasswordWidget* PasswordWidget = CreateWidget<UEnterPasswordWidget>(GetWorld(), PasswordWidgetClass))
        {
            PasswordWidget->Init(RoomInfo.RoomID);
            PasswordWidget->AddToViewport();
        }*/
    }
    else
    {
        if (ALCLobbyPlayerController* PC = GetOwningPlayer<ALCLobbyPlayerController>())
        {
            PC->Server_RequestJoinRoom(RoomInfo.RoomID, TEXT(""));
        }
    }
}
