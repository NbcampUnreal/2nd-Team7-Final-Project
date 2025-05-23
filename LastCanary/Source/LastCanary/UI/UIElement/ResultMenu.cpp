#include "UI/UIElement/ResultMenu.h"
#include "UI/UIObject/RewardEntry.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

void UResultMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (AcceptButton)
	{
		AcceptButton->OnClicked.AddUniqueDynamic(this, &UResultMenu::OnAcceptClicked);
	}
}

void UResultMenu::NativeDestruct()
{
	Super::NativeDestruct();
	if (AcceptButton)
	{
		AcceptButton->OnClicked.RemoveDynamic(this, &UResultMenu::OnAcceptClicked);
	}
}

void UResultMenu::SetRewardEntries(const TArray<FResultRewardEntry>& InEntries)
{
    CachedEntries = InEntries;

    if (!RewardScrollBox)
    {
        return;
    }
    RewardScrollBox->ClearChildren();

    for (const FResultRewardEntry& Entry : CachedEntries)
    {
        if (!RewardEntryClass)
        {
            continue;
        }
        URewardEntry* EntryWidget = CreateWidget<URewardEntry>(this, RewardEntryClass);
        if (EntryWidget)
        {
            EntryWidget->InitWithEntry(Entry);
            RewardScrollBox->AddChild(EntryWidget);
        }
    }

    LOG_Frame_WARNING(TEXT("ResultMenu - Reward Entries Displayed: %d"), CachedEntries.Num());
}

void UResultMenu::SetTotalGold(int32 InTotalGold)
{
    if (TotalGoldText)
    {
        TotalGoldText->SetText(FText::AsNumber(InTotalGold));
    }
}

void UResultMenu::OnAcceptClicked()
{
    RemoveFromParent();

    LOG_Frame_WARNING(TEXT("ResultMenu - AcceptClicked → Returning to BaseCamp"));

    // TODO : 레벨 전환
    // UGameplayStatics::OpenLevel(this, FName("L_BaseCamp"));
}

void UResultMenu::ActivateResultCamera()
{
    // 선택 사항: 카메라 ViewTarget 변경 또는 SceneCapture 등
}