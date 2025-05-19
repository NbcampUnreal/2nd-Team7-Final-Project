// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "DataType/ToolbarItemSlotData.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "UI/UIObject/ToolbarSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "LastCanary.h"

void UToolbarInventoryWidget::RefreshInventoryUI()
{
	if (!InventoryComponent)
	{
		LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] InventoryComponent is null!"));
		return;
	}

	if (!ItemDataTable)
	{
		LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] ItemDataTable is null!"));
		return;
	}

	if (!ToolbarSlotBox)
	{
		LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] ToolbarSlotBox is null!"));
		return;
	}

	if (!SlotWidgetClass)
	{
		LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] SlotWidgetClass is null!"));
	}

	ToolbarSlotBox->ClearChildren();

	UToolbarInventoryComponent* ToolbarInventory = Cast<UToolbarInventoryComponent>(InventoryComponent);
	if (!ToolbarInventory)
	{
		LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] InventoryComponent 캐스팅 실패!"));
		return;
	}

	for (int32 i = 0; i < ToolbarInventory->ItemSlots.Num(); ++i)
	{
		const FBaseItemSlotData& NewSlot = InventoryComponent->ItemSlots[i];
		UToolbarSlotWidget* SlotWidget = CreateWidget<UToolbarSlotWidget>(this, SlotWidgetClass);

		SlotWidget->SetItemData(NewSlot, InventoryComponent->ItemDataTable);

		//const FBaseItemSlotData& ItemSlot = ToolbarInventory->ItemSlots[i];
		//UToolbarSlotWidget* SlotWidget = CreateWidget<UToolbarSlotWidget>(this, SlotWidgetClass);

		//// 슬롯 위젯에 데이터 및 아이템 액터 설정
		//SlotWidget->SetItemData(ItemSlot, ItemDataTable);
		////SlotWidget->SetItemActor(ToolbarInventory->SlotIndexToItemActor.FindRef(i));
	}
}
