#include "UI/UIObject/BackpackSlotWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/PanelWidget.h"
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

    bool bHandled = false;

    if (SourceBackpackWidget)
    {
        // 가방 슬롯 간 스왑
        bHandled = ToolbarInventory->TrySwapBackpackSlots(
            SourceBackpackWidget->BackpackSlotIndex,
            this->BackpackSlotIndex
        );

        LOG_Item_WARNING(TEXT("[BackpackSlotWidget::NativeOnDrop] 가방 슬롯 교체 결과: %s (from %d to %d)"),
            bHandled ? TEXT("성공") : TEXT("실패"),
            SourceBackpackWidget->BackpackSlotIndex,
            this->BackpackSlotIndex);
    }
    else
    {
        // 툴바에서 가방으로 아이템 이동
        bHandled = ToolbarInventory->TryMoveToolbarItemToBackpack(
            SourceWidget->SlotIndex,
            this->BackpackSlotIndex
        );

        LOG_Item_WARNING(TEXT("[BackpackSlotWidget::NativeOnDrop] 툴바->가방 이동 결과: %s (from toolbar %d to backpack %d)"),
            bHandled ? TEXT("성공") : TEXT("실패"),
            SourceWidget->SlotIndex,
            this->BackpackSlotIndex);
    }

    // ⭐ 기존 처리가 실패했거나 빈 슬롯이면 외부 드롭으로 처리
    if (!bHandled)
    {
        // 현재 슬롯이 비어있고, 드롭 위치가 슬롯 영역 밖이면 외부 드롭으로 간주
        if (ItemData.ItemRowName.IsNone() || ItemData.ItemRowName == FName("Default"))
        {
            return HandleExternalDrop(SourceWidget);
        }
    }

    return bHandled;
}

void UBackpackSlotWidget::NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation)
{
    if (UInventoryUtility::IsDefaultItem(ItemData.ItemRowName, InventoryComponent ? InventoryComponent->GetInventoryConfig() : nullptr))
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

UInventoryMainWidget* UBackpackSlotWidget::GetInventoryMainWidget() const
{
    // 1. 직접적인 부모에서 찾기
    UInventoryMainWidget* MainWidget = GetTypedOuter<UInventoryMainWidget>();
    if (MainWidget)
    {
        return MainWidget;
    }

    // 2. 위젯 트리를 거슬러 올라가며 찾기
    UWidget* CurrentWidget = GetParent();
    while (CurrentWidget)
    {
        if (UInventoryMainWidget* FoundMainWidget = Cast<UInventoryMainWidget>(CurrentWidget))
        {
            return FoundMainWidget;
        }
        CurrentWidget = CurrentWidget->GetParent();
    }

    // 3. 월드의 모든 위젯에서 찾기 (최후의 수단)
    if (UWorld* World = GetWorld())
    {
        for (TObjectIterator<UInventoryMainWidget> It; It; ++It)
        {
            if (It->GetWorld() == World && IsValid(*It))
            {
                return *It;
            }
        }
    }

    return nullptr;
}

// ⭐ 외부 드롭 처리 함수
bool UBackpackSlotWidget::HandleExternalDrop(UInventorySlotWidget* SourceWidget)
{
    if (!SourceWidget)
    {
        return false;
    }

    // Default 아이템이면 드롭 불가
    if (SourceWidget->ItemData.ItemRowName.IsNone() ||
        SourceWidget->ItemData.ItemRowName == FName("Default"))
    {
        return false;
    }

    UInventoryMainWidget* MainWidget = GetInventoryMainWidget();
    if (!MainWidget)
    {
        LOG_Item_WARNING(TEXT("[BackpackSlotWidget::HandleExternalDrop] InventoryMainWidget을 찾을 수 없음"));
        return false;
    }

    // 개수에 따라 처리 분기
    if (SourceWidget->ItemData.Quantity <= 1)
    {
        // 1개면 바로 드롭
        MainWidget->HandleDropOutsideSlots(SourceWidget, 1);
    }
    else
    {
        // 여러 개면 개수 선택 UI 표시
        MainWidget->ShowItemDropQuantityWidget(SourceWidget);
    }

    LOG_Item_WARNING(TEXT("[BackpackSlotWidget::HandleExternalDrop] 외부 드롭 처리 완료"));
    return true;
}