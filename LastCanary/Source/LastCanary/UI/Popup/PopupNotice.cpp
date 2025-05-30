#include "UI/Popup/PopupNotice.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UPopupNotice::NativeConstruct()
{
	Super::NativeConstruct();
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UPopupNotice::OnExitButtonClicked);
	}
}

void UPopupNotice::NativeDestruct()
{
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UPopupNotice::OnExitButtonClicked);
	}
}

void UPopupNotice::InitializeNoticePopup(const FString& Notice)
{
	if (NoticeText)
	{
		NoticeText->SetText(FText::FromString(Notice));
	}
}

void UPopupNotice::OnExitButtonClicked()
{
	RemoveFromParent();
	Destruct();
}
