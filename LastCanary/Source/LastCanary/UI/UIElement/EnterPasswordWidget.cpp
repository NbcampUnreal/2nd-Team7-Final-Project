#include "UI/UIElement/EnterPasswordWidget.h"
#include "Components/EditableTextBox.h"
#include "Components/Button.h"
//#include "Framework/PlayerController/LCLobbyPlayerController.h"

void UEnterPasswordWidget::Init(const FString& InRoomID)
{
    RoomID = InRoomID;
}

void UEnterPasswordWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.AddUniqueDynamic(this, &UEnterPasswordWidget::OnConfirmButtonClicked);
    }
}

void UEnterPasswordWidget::NativeDestruct()
{
    Super::NativeDestruct();
    if (ConfirmButton)
    {
        ConfirmButton->OnClicked.RemoveDynamic(this, &UEnterPasswordWidget::OnConfirmButtonClicked);
    }
}

void UEnterPasswordWidget::OnConfirmButtonClicked()
{
    if (PasswordInput == nullptr)
    {
        return;
    }

    FString EnteredPassword = PasswordInput->GetText().ToString();

    //if (ALCLobbyPlayerController* PC = Cast<ALCLobbyPlayerController>(GetOwningPlayer()))
    //{
    //    PC->Server_RequestJoinRoom(RoomID, EnteredPassword);
    //}

    SetVisibility(ESlateVisibility::Hidden);
}
