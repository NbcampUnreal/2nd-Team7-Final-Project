#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "UI/UIObject/ToolbarSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"

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
        LOG_Item_ERROR(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] SlotWidgetClass is null! 블루프린트에서 설정하세요."));
        return;
    }

    ToolbarSlotBox->ClearChildren();

    UToolbarInventoryComponent* ToolbarInventory = Cast<UToolbarInventoryComponent>(InventoryComponent);
    if (!ToolbarInventory)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] InventoryComponent 캐스팅 실패!"));
        return;
    }

    for (int32 i = 0; i < ToolbarInventory->GetMaxSlots(); ++i)
    {
        const FBaseItemSlotData& SlotData = ToolbarInventory->ItemSlots[i];

        UInventorySlotWidget* SlotWidget = CreateSlotWidget(i, SlotData);
        if (!SlotWidget)
        {
            continue;
        }

        ToolbarSlotBox->AddChild(SlotWidget);
    }
}
