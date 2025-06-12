#include "UI/UIElement/BackpackInventoryWidget.h"
#include "UI/UIObject/BackpackSlotWidget.h"
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

    if (!BackpackSlotWidgetClass)
    {
        LOG_Item_ERROR(TEXT("[BackpackInventoryWidget::RefreshInventoryUI] BackpackSlotWidgetClass is null!"));
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
        UBackpackSlotWidget* SlotWidget = CreateBackpackSlotWidget(i, BackpackSlots[i]);
        if (!SlotWidget)
        {
            continue;
        }

        int32 Row = i / ColumnsPerRow;
        int32 Col = i % ColumnsPerRow;

        BackpackSlotPanel->AddChildToGrid(SlotWidget, Row, Col);
    }
}

UBackpackSlotWidget* UBackpackInventoryWidget::CreateBackpackSlotWidget(int32 BackpackSlotIndex, const FBackpackSlotData& SlotData)
{
    if (!BackpackSlotWidgetClass)
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackInventoryWidget::CreateBackpackSlotWidget] BackpackSlotWidgetClass가 설정되지 않음"));
        return nullptr;
    }

    UBackpackSlotWidget* SlotWidget = CreateWidget<UBackpackSlotWidget>(this, BackpackSlotWidgetClass);
    if (!SlotWidget)
    {
        UE_LOG(LogTemp, Error, TEXT("[BackpackInventoryWidget::CreateBackpackSlotWidget] 가방 슬롯 위젯 생성 실패: %d"), BackpackSlotIndex);
        return nullptr;
    }

    // 가방 슬롯 데이터를 베이스 슬롯 데이터로 변환
    FBaseItemSlotData BaseSlot = ConvertBackpackSlotToBaseSlot(SlotData);

    // 슬롯 데이터 설정
    SlotWidget->SetItemData(BaseSlot, ItemDataTable);
    SlotWidget->SetInventoryComponent(InventoryComponent);
    SlotWidget->SlotIndex = -1; // 툴바 인덱스는 사용하지 않음
    SlotWidget->BackpackSlotIndex = BackpackSlotIndex; // 가방 인덱스 설정

    // 부모 인벤토리 위젯 참조 설정
    SlotWidget->SetParentInventoryWidget(this);

    return SlotWidget;
}