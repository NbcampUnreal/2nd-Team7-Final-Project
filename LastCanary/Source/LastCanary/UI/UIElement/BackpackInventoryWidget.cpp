#include "UI/UIElement/BackpackInventoryWidget.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "Components/GridPanel.h"
#include "LastCanary.h"

void UBackpackInventoryWidget::RefreshInventoryUI()
{
	if (!InventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] InventoryComponent is null!"));
		return;
	}

	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] ItemDataTable is null!"));
		return;
	}

	if (!BackpackSlotPanel)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackSlotPanel is null!"));
		return;
	}

	if (!SlotWidgetClass)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] SlotWidgetClass is null!"));
		return;
	}

	BackpackSlotPanel->ClearChildren();

	UBackpackInventoryComponent* BackpackInventory = Cast<UBackpackInventoryComponent>(InventoryComponent);
	if (!BackpackInventory)
	{
		LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackInventory is null!"));
		return;
	}

	for (int32 i = 0; i < BackpackInventory->ItemSlots.Num(); ++i)
	{
		const FBaseItemSlotData& ItemSlot = BackpackInventory->ItemSlots[i];

		UInventorySlotWidget* SlotWidget = CreateWidget<UInventorySlotWidget>(this, SlotWidgetClass);
		if (!SlotWidget)
		{
			LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] SlotWidget is null!"));
			return;
		}

		FBaseItemSlotData InstanceData;
		InstanceData.ItemRowName = ItemSlot.ItemRowName;
		InstanceData.Quantity = ItemSlot.Quantity;

		SlotWidget->SetItemData(InstanceData, ItemDataTable);
		SlotWidget->SlotIndex = i;
		SlotWidget->InventoryComponent = InventoryComponent;
		BackpackSlotPanel->AddChild(SlotWidget);
	}
}
