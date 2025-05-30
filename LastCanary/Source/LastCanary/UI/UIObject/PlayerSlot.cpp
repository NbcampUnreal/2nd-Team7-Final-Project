#include "UI/UIObject/PlayerSlot.h"
#include "Components/TextBlock.h"
#include "DataType/SessionPlayerInfo.h"

void UPlayerSlot::SetSlotIndex(const int Index)
{
    if (IndexText)
    {
        IndexText->SetText(FText::AsNumber(Index));
    }
}

void UPlayerSlot::SetPlayerInfo(const FSessionPlayerInfo& PlayerInfo)
{
    if (NickNameText)
    {
        NickNameText->SetText(FText::FromString(PlayerInfo.PlayerName));
    }
}
