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

    UBackpackInventoryComponent* BackpackInventory = Cast<UBackpackInventoryComponent>(InventoryComponent);
    if (!BackpackInventory)
    {
        LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackInventory Cast 실패!"));
        LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] InventoryComponent 클래스: %s"),
            InventoryComponent ? *InventoryComponent->GetClass()->GetName() : TEXT("NULL"));
        return;
    }

    for (int32 i = 0; i < BackpackInventory->ItemSlots.Num(); ++i)
    {
        const FBaseItemSlotData& SlotData = BackpackInventory->ItemSlots[i];

        UInventorySlotWidget* SlotWidget = CreateSlotWidget(i, SlotData);
        if (!SlotWidget)
        {
            LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] 슬롯 위젯 생성 실패: %d"), i);
            continue;
        }

        int32 Row = i / ColumnsPerRow;
        int32 Column = i % ColumnsPerRow;

        BackpackSlotPanel->AddChildToGrid(SlotWidget, Row, Column);
    }
}
