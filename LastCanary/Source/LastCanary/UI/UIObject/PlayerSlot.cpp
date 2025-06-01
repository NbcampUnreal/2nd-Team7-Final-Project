#include "UI/UIObject/PlayerSlot.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "DataType/SessionPlayerInfo.h"
#include "GameFramework/PlayerState.h"

void UPlayerSlot::NativeConstruct()
{
    Super::NativeConstruct();

}

void UPlayerSlot::NativeDestruct()
{
    Super::NativeDestruct();

}

void UPlayerSlot::SetSlotIndex(const int Index)
{
    if (IndexText)
    {
        IndexText->SetText(FText::AsNumber(Index));
    }
}

void UPlayerSlot::SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo)
{
    MySessionPlayerInfo = PlayerInfo;

    if (NickNameText)
    {
        NickNameText->SetText(FText::FromString(PlayerInfo.PlayerName));
    }

    if (ReadyText)
    {
        FString IsReady = PlayerInfo.bIsPlayerReady ? "Ready" : "Not Ready";
        ReadyText->SetText(FText::FromString(IsReady));
    }

    if (!GetOwningPlayer()->HasAuthority())
    {
        KickButton->SetVisibility(ESlateVisibility::Hidden);
    }
    else
    {
        if (APlayerState* PS = GetOwningPlayer()->PlayerState)
        {
            if (PlayerInfo.PlayerName == PS->GetPlayerName())
            {
                KickButton->SetVisibility(ESlateVisibility::Hidden);
            }
            else
            {
                KickButton->SetVisibility(ESlateVisibility::Visible);
            }
        }
    }
}

void UPlayerSlot::ClearPlayerInfo()
{
    NickNameText->SetText(FText::FromString("Empty Slot"));
}
