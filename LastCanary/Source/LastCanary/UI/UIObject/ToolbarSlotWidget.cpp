#include "UI/UIObject/ToolbarSlotWidget.h"
#include "LastCanary.h"

void UToolbarSlotWidget::SetItemActor(AItemBase* NewItemActor)
{
	if (!NewItemActor)
	{
		LOG_Item_WARNING(TEXT("[UToolbarSlotWidget::SetItemActor] NewItemActor is null!"));
		return;
	}

	ItemActor = NewItemActor;
}
