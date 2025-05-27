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
        LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackSlotPanel is null! BindWidget 확인하세요."));
        return;
    }

    if (!SlotWidgetClass)
    {
        LOG_Item_ERROR(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] SlotWidgetClass is null! 블루프린트에서 설정하세요."));
        return;
    }

    // 기존 슬롯들 제거
    BackpackSlotPanel->ClearChildren();

    UBackpackInventoryComponent* BackpackInventory = Cast<UBackpackInventoryComponent>(InventoryComponent);
    if (!BackpackInventory)
    {
        LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackInventory is null!"));
        return;
    }

    // 슬롯 위젯들 생성
    for (int32 i = 0; i < BackpackInventory->ItemSlots.Num(); ++i)
    {
        const FBaseItemSlotData& SlotData = BackpackInventory->ItemSlots[i];

        // ⭐ 부모 클래스의 헬퍼 함수 사용
        UInventorySlotWidget* SlotWidget = CreateSlotWidget(i, SlotData);
        if (!SlotWidget)
        {
            continue;
        }

        // 그리드 위치 계산
        int32 Row = i / ColumnsPerRow;
        int32 Column = i % ColumnsPerRow;

        // 그리드 패널에 추가
        BackpackSlotPanel->AddChildToGrid(SlotWidget, Row, Column);
    }

    LOG_Item_WARNING(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] UI 새로고침 완료 - 총 %d개 슬롯"),
        BackpackInventory->ItemSlots.Num());
}
