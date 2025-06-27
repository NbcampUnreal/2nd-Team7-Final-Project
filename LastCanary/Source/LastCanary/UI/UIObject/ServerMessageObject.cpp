#include "UI/UIObject/ServerMessageObject.h"

#include "Components/TextBlock.h"

#include "LastCanary.h"

void UServerMessageObject::NativeConstruct()
{
	Super::NativeConstruct();

	if (DisplayMessageAnim)
	{
		//LOG_Frame_WARNING(TEXT("ChecklistWidget - RevealSignatureAnim 재생 시작"));
		FWidgetAnimationDynamicEvent AnimFinishedDelegate;
		AnimFinishedDelegate.BindDynamic(this, &UServerMessageObject::OnDisplayMessageAnimFinished);
		BindToAnimationFinished(DisplayMessageAnim, AnimFinishedDelegate);
		PlayAnimation(DisplayMessageAnim);
	}
	else
	{
		OnDisplayMessageAnimFinished();
	}
}

void UServerMessageObject::NativeDestruct()
{
	Super::NativeDestruct();

}

void UServerMessageObject::OnDisplayMessageAnimFinished()
{
	RemoveFromParent();
}

void UServerMessageObject::SetMessage(const FString& Message)
{
	if (MessageText)
	{
		MessageText->SetText(FText::FromString(Message));
	}
}
