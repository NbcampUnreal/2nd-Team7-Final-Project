#include "UI/UIObject/PlayerSlot.h"
#include "Components/TextBlock.h"

void UPlayerSlot::SetPlayerInfo(int Index, FString Name)
{
    if (IndexText)
    {
        IndexText->SetText(FText::AsNumber(Index));
    }
    if (NickNameText)
    {
        NickNameText->SetText(FText::FromString(Name));
    }
}

void UPlayerSlot::SetPlayerName(FString Name)
{
}
