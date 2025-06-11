#include "UI/UIElement/BackpackInventoryWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Components/GridPanel.h"
#include "LastCanary.h"

void UBackpackInventoryWidget::RefreshInventoryUI()
{
    if (!InventoryComponent)
    {
        LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] InventoryComponent is null!"));
        return;
    }

    if (!BackpackSlotPanel)
    {
        LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackSlotPanel is null!"));
        return;
    }

    if (!SlotWidgetClass)
    {
        LOG_Item_ERROR(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] SlotWidgetClass is null!"));
        return;
    }

    BackpackSlotPanel->ClearChildren();

    UToolbarInventoryComponent* ToolbarInventory = Cast<UToolbarInventoryComponent>(InventoryComponent);
    if (!ToolbarInventory)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] InventoryComponent 캐스팅 실패!"));
        return;
    }

    TArray<FBackpackSlotData> BackpackSlots = ToolbarInventory->GetCurrentBackpackSlots();
    for (int32 i = 0; i < BackpackSlots.Num(); ++i)
    {
        FBaseItemSlotData BaseSlot = ConvertBackpackSlotToBaseSlot(BackpackSlots[i]);
        UInventorySlotWidget* SlotWidget = CreateSlotWidget(i, BaseSlot);
        if (!SlotWidget)
        {
            continue;
        }

        int32 Row = i / ColumnsPerRow;
        int32 Col = i % ColumnsPerRow;

        BackpackSlotPanel->AddChildToGrid(SlotWidget, Row, Col);
    }
}
