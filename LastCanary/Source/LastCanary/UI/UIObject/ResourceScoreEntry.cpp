#include "UI/UIObject/ResourceScoreEntry.h"
#include "Components/TextBlock.h"

void UResourceScoreEntry::NativeConstruct()
{
    Super::NativeConstruct();

    if (RevealEntryAnim)
    {
        PlayAnimation(RevealEntryAnim);
    }
}

void UResourceScoreEntry::NativeDestruct()
{
    Super::NativeDestruct();
}

void UResourceScoreEntry::InitResourceScoreEntry(const FText& ResourceName, const FText& ScoreDescription)
{
    if (ResourceNameText)
    {
        ResourceNameText->SetText(ResourceName);
    }

    if (ScoreDetailText)
    {
        ScoreDetailText->SetText(ScoreDescription);
    }
}
