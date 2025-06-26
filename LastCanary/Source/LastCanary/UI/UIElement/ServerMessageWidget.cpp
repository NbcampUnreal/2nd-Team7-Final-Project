#include "UI/UIElement/ServerMessageWidget.h"
#include "UI/UIObject/ServerMessageObject.h"

#include "Components/ScrollBox.h"

void UServerMessageWidget::NativeConstruct()
{
	Super::NativeConstruct();

}

void UServerMessageWidget::NativeDestruct()
{
	Super::NativeDestruct();

}

void UServerMessageWidget::AddMessage(const FString& Message)
{
	if (MessageScrollBox)
	{
		UServerMessageObject* MessageObject = CreateWidget<UServerMessageObject>(this, MessageObjectClass);
		if (MessageObject)
		{
			MessageScrollBox->AddChild(MessageObject);
			MessageObject->SetMessage(Message);
		}
	}
}
