#include "UI/UIObject/ItemContainerSlotWidget.h"
#include "UI/UIElement/ItemContainerWidget.h"
#include "UI/UIObject/BackpackSlotWidget.h"
#include "Inventory/InventoryUtility.h"

#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
#include "LastCanary.h"

bool UItemContainerSlotWidget::NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation)
{
    UInventorySlotWidget* SourceWidget = Cast<UInventorySlotWidget>(InOperation->Payload);
    if (!SourceWidget)
    {
        return false;
    }

    if (SourceWidget == this)
    {
        return true;
    }

    if (!ContainerWidget)
    {
        LOG_Item_WARNING(TEXT("[ContainerSlotWidget] ContainerWidget is null"));
        return false;
    }

    return HandleDragDropOperation(SourceWidget);
}

void UItemContainerSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    // Default 아이템이면 드래그 불가
    if (UInventoryUtility::IsDefaultItem(ItemData.ItemRowName))
    {
        OutOperation = nullptr;
        return;
    }

    if (ItemData.Quantity <= 0)
    {
        OutOperation = nullptr;
        return;
    }

    UDragDropOperation* DragOp = NewObject<UDragDropOperation>();

    // 컨테이너 슬롯용 드래그 비주얼 생성
    UItemContainerSlotWidget* DragVisual = CreateWidget<UItemContainerSlotWidget>(GetWorld(), GetClass());
    if (DragVisual)
    {
        DragVisual->SetItemData(ItemData, ItemDataTable);
        DragVisual->ContainerSlotIndex = this->ContainerSlotIndex;
        DragVisual->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

        // 테두리 제거
        if (DragVisual->SlotBorder)
        {
            FSlateBrush EmptyBrush;
            EmptyBrush.DrawAs = ESlateBrushDrawType::NoDrawType;
            DragVisual->SlotBorder->SetBrush(EmptyBrush);
        }
    }

    DragOp->DefaultDragVisual = DragVisual;
    DragOp->Payload = this;

    OutOperation = DragOp;
}

bool UItemContainerSlotWidget::HandleDragDropOperation(UInventorySlotWidget* SourceWidget)
{
    if (!SourceWidget || !ContainerWidget)
    {
        return false;
    }

    // 컨테이너 슬롯인지 확인
    UItemContainerSlotWidget* SourceContainerWidget = Cast<UItemContainerSlotWidget>(SourceWidget);

    if (SourceContainerWidget)
    {
        // 컨테이너 슬롯 간 이동
        return HandleContainerToContainerDrop(SourceContainerWidget);
    }
    else
    {
        // 플레이어 인벤토리에서 컨테이너로 이동
        return HandlePlayerToContainerDrop(SourceWidget);
    }
}

bool UItemContainerSlotWidget::HandleContainerToContainerDrop(UItemContainerSlotWidget* SourceWidget)
{
    if (!SourceWidget || !ContainerWidget)
    {
        return false;
    }

    // 빈 슬롯인 경우 아이템 이동
    if (!ItemData.bIsValid || UInventoryUtility::IsDefaultItem(ItemData.ItemRowName))
    {
        if (CanPlaceItemInEmptySlot(SourceWidget->ItemData))
        {
            ContainerWidget->HandleContainerSlotSwap(SourceWidget, this->ContainerSlotIndex);
            return true;
        }
    }
    // 동일한 아이템인 경우 스택 시도
    else if (CanStackWithItem(SourceWidget->ItemData))
    {
        ContainerWidget->HandleContainerSlotSwap(SourceWidget, this->ContainerSlotIndex);
        return true;
    }
    // 다른 아이템인 경우 스왑
    else
    {
        ContainerWidget->HandleContainerSlotSwap(SourceWidget, this->ContainerSlotIndex);
        return true;
    }

    return false;
}

bool UItemContainerSlotWidget::HandlePlayerToContainerDrop(UInventorySlotWidget* SourceWidget)
{
    if (!SourceWidget || !ContainerWidget)
    {
        return false;
    }

    // 플레이어 인벤토리에서 컨테이너로 아이템 이동
    ContainerWidget->HandleItemMoveToContainer(SourceWidget, this->ContainerSlotIndex);

    LOG_Item_WARNING(TEXT("[ContainerSlotWidget] 플레이어 -> 컨테이너 드롭 처리: %s"),
        *SourceWidget->ItemData.ItemRowName.ToString());

    return true;
}

bool UItemContainerSlotWidget::CanPlaceItemInEmptySlot(const FBaseItemSlotData& SourceItemData) const
{
    // 빈 슬롯이고, 소스 아이템이 유효하면 배치 가능
    return (!ItemData.bIsValid || UInventoryUtility::IsDefaultItem(ItemData.ItemRowName)) &&
        SourceItemData.bIsValid && !UInventoryUtility::IsDefaultItem(SourceItemData.ItemRowName);
}

bool UItemContainerSlotWidget::CanStackWithItem(const FBaseItemSlotData& SourceItemData) const
{
    // 동일한 아이템이고, 둘 다 유효하면 스택 가능
    return ItemData.bIsValid && SourceItemData.bIsValid &&
        ItemData.ItemRowName == SourceItemData.ItemRowName &&
        !UInventoryUtility::IsDefaultItem(ItemData.ItemRowName);
}
