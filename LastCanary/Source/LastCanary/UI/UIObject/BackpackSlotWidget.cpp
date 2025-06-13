#include "UI/UIObject/BackpackSlotWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "LastCanary.h"

bool UBackpackSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UInventorySlotWidget* SourceWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
    if (!SourceWidget || SourceWidget == this)
    {
        return false;
    }

    UToolbarInventoryComponent* ToolbarInventory = Cast<UToolbarInventoryComponent>(InventoryComponent);
    if (!ToolbarInventory)
    {
        LOG_Item_WARNING(TEXT("[BackpackSlotWidget::NativeOnDrop] ToolbarInventoryComponent is null"));
        return false;
    }

    // 소스가 가방 슬롯인지 툴바 슬롯인지 확인
    UBackpackSlotWidget* SourceBackpackWidget = Cast<UBackpackSlotWidget>(SourceWidget);

    if (SourceBackpackWidget)
    {
        // 가방 슬롯 간 스왑
        bool bSuccess = ToolbarInventory->TrySwapBackpackSlots(
            SourceBackpackWidget->BackpackSlotIndex,
            this->BackpackSlotIndex
        );

        LOG_Item_WARNING(TEXT("[BackpackSlotWidget::NativeOnDrop] 가방 슬롯 교체 결과: %s (from %d to %d)"),
            bSuccess ? TEXT("성공") : TEXT("실패"),
            SourceBackpackWidget->BackpackSlotIndex,
            this->BackpackSlotIndex);

        return bSuccess;
    }
    else
    {
        // 툴바에서 가방으로 아이템 이동
        bool bSuccess = ToolbarInventory->TryMoveToolbarItemToBackpack(
            SourceWidget->SlotIndex,
            this->BackpackSlotIndex
        );

        LOG_Item_WARNING(TEXT("[BackpackSlotWidget::NativeOnDrop] 툴바->가방 이동 결과: %s (from toolbar %d to backpack %d)"),
            bSuccess ? TEXT("성공") : TEXT("실패"),
            SourceWidget->SlotIndex,
            this->BackpackSlotIndex);

        return bSuccess;
    }
}

void UBackpackSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

    // 가방 슬롯용 드래그 비주얼 생성
    UBackpackSlotWidget* DragVisual = CreateWidget<UBackpackSlotWidget>(GetWorld(), GetClass());
    if (DragVisual)
    {
        DragVisual->SetItemData(ItemData, ItemDataTable);
        DragVisual->BackpackSlotIndex = this->BackpackSlotIndex;
        DragVisual->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
    }

    DragOp->DefaultDragVisual = DragVisual;
    DragOp->Payload = this;

    OutOperation = DragOp;
}