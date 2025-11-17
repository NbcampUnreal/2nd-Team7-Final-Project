#include "UI/UIObject/RewardEntry.h"
#include "Components/TextBlock.h"

void URewardEntry::NativeConstruct()
{
    Super::NativeConstruct();

    if (RevealEntryAnim)
    {
        PlayAnimation(RevealEntryAnim);
    }
}

void URewardEntry::NativeDestruct()
{
    Super::NativeDestruct();
}

void URewardEntry::InitWithEntry(const FResultRewardEntry& Entry)
{
    if (DescriptionText)
    {
        DescriptionText->SetText(Entry.Description);
    }

    if (GoldText)
    {
        GoldText->SetText(FText::AsNumber(Entry.RewardGold));
    }
}
