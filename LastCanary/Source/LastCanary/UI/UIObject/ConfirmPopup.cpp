#include "UI/UIObject/ConfirmPopup.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UConfirmPopup::Init(TFunction<void()> OnConfirm)
{
	//외부에서 받은 함수 콜백을 멤버 변수에 저장
	ConfirmCallback = MoveTemp(OnConfirm);
}

void UConfirmPopup::NativeConstruct()
{
	Super::NativeConstruct();
	if (YesButton)
	{
		YesButton->OnClicked.AddUniqueDynamic(this, &UConfirmPopup::OnYesClicked);
	}
	if (NoButton)
	{
		NoButton->OnClicked.AddUniqueDynamic(this, &UConfirmPopup::OnNoClicked);
	}
}

void UConfirmPopup::NativeDestruct()
{
	Super::NativeDestruct();
	if (YesButton)
	{
		YesButton->OnClicked.RemoveDynamic(this, &UConfirmPopup::OnYesClicked);
	}
	if (NoButton)
	{
		NoButton->OnClicked.RemoveDynamic(this, &UConfirmPopup::OnNoClicked);
	}
}

void UConfirmPopup::OnYesClicked()
{
	if (ConfirmCallback)
	{
		ConfirmCallback();
	}
	this->RemoveFromParent();
}

void UConfirmPopup::OnNoClicked()
{
	this->RemoveFromParent();
}