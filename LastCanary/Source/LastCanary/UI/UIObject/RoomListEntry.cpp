#include "UI/UIObject/RoomListEntry.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "OnlineSubsystem.h"

#include "Framework/PlayerController/LCLobbyPlayerController.h"

void URoomListEntry::NativeConstruct()
{
	Super::NativeConstruct();
    // BluePrint에서 온클릭 이벤트 바인딩
	//if (JoinButton)
	//{
	//	JoinButton->OnClicked.AddUniqueDynamic(this, &URoomListEntry::OnJoinButtonClicked);
    //}

    SetRoomListInfo();
}

void URoomListEntry::NativeDestruct()
{
    Super::NativeDestruct();
    //if (JoinButton)
    //{
    //    JoinButton->OnClicked.RemoveDynamic(this, &URoomListEntry::OnJoinButtonClicked);
	//}
}

void URoomListEntry::SetRoomListInfo_Implementation()
{
}

void URoomListEntry::OnJoinButtonClicked()
{

    //if (RoomInfo.bIsPrivate)
    //{
    //    /*if (UEnterPasswordWidget* PasswordWidget = CreateWidget<UEnterPasswordWidget>(GetWorld(), PasswordWidgetClass))
    //    {
    //        PasswordWidget->Init(RoomInfo.RoomID);
    //        PasswordWidget->AddToViewport();
    //    }*/
    //}
    //else
    //{
    //    if (ALCLobbyPlayerController* PC = GetOwningPlayer<ALCLobbyPlayerController>())
    //    {
    //        PC->Server_RequestJoinRoom(RoomInfo.RoomID, TEXT(""));
    //    }
    //}
}
