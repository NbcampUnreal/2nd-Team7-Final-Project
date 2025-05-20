#include "UI/UIElement/ResultMenu.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"


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

    if (!RewardScrollBox) return;
    RewardScrollBox->ClearChildren();

    for (const FResultRewardEntry& Entry : CachedEntries)
    {
        // 보상 항목 위젯 생성 (ResultEntry는 따로 만들어야 함)
        //UResultEntryWidget* EntryWidget = CreateWidget<UResultEntryWidget>(this, UResultEntryWidget::StaticClass());
        //if (EntryWidget)
        //{
        //    EntryWidget->InitWithEntry(Entry); // 커스텀 Init 함수 필요
        //    RewardScrollBox->AddChild(EntryWidget);
        //}
    }
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
    // 여기서 결과 화면 닫기, 서버에 결과 전송 등 처리
    RemoveFromParent();
    // 예: UGameplayStatics::OpenLevel(...)로 로비 이동 등도 가능
}

void UResultMenu::ActivateResultCamera()
{
    // 예시: Result 전용 카메라를 활성화할 때 사용하는 로직 (선택 사항)
    // 이 기능이 필요하면 PlayerController에서 ViewTarget 전환하거나
    // SceneCapture2D를 사용해서 UI에서 카메라 결과를 보여주면 됨
}