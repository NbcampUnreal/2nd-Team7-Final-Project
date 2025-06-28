#include "UI/UIObject/BackpackSlotWidget.h"
#include "UI/UIElement/InventoryMainWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Inventory/InventoryUtility.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Components/Border.h"
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

    bool bHandled = TryStackOrSwap(SourceWidget, ToolbarInventory);

    // 처리 실패 시 외부 드롭으로 처리
    if (!bHandled)
    {
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

bool UBackpackSlotWidget::TryStackOrSwap(UInventorySlotWidget* SourceWidget, UToolbarInventoryComponent* ToolbarInventory)
{
    if (!SourceWidget || !ToolbarInventory->BackpackManager)
    {
        return false;
    }

    UBackpackSlotWidget* SourceBackpackWidget = Cast<UBackpackSlotWidget>(SourceWidget);

    // 스택 가능한지 확인하고 시도
    if (CanStack(SourceWidget->ItemData, this->ItemData))
    {
        int32 MovedQuantity = 0;

        if (SourceBackpackWidget)
        {
            // 가방 슬롯 간 스택 시도
            MovedQuantity = ToolbarInventory->BackpackManager->MoveAndStack(
                SourceBackpackWidget->BackpackSlotIndex,
                this->BackpackSlotIndex,
                SourceWidget->ItemData.Quantity
            );
        }
        else
        {
            // 툴바에서 가방으로 스택 시도
            const FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(
                SourceWidget->ItemData.ItemRowName,
                TEXT("TryStackOrSwap")
            );

            if (ItemRowData)
            {
                FBaseItemSlotData& BackpackOwnerSlot = ToolbarInventory->ItemSlots[ToolbarInventory->BackpackManager->CurrentBackpackSlotIndex];
                if (BackpackOwnerSlot.BackpackSlots.IsValidIndex(this->BackpackSlotIndex))
                {
                    bool bStackSuccess = ToolbarInventory->BackpackManager->AddToSlot(
                        BackpackOwnerSlot.BackpackSlots[this->BackpackSlotIndex],
                        SourceWidget->ItemData.ItemRowName,
                        SourceWidget->ItemData.Quantity,
                        ItemRowData->MaxStack
                    );

                    if (bStackSuccess)
                    {
                        // 툴바 슬롯 비우기
                        UInventoryUtility::SetSlotToDefault(
                            ToolbarInventory->ItemSlots[SourceWidget->SlotIndex],
                            ToolbarInventory->GetInventoryConfig()
                        );
                        ToolbarInventory->OnInventoryUpdated.Broadcast();
                        MovedQuantity = SourceWidget->ItemData.Quantity;
                    }
                }
            }
        }

        if (MovedQuantity > 0)
        {
            LOG_Item_WARNING(TEXT("[TryStackOrSwap] 스택 성공: %d개 이동"), MovedQuantity);
            return true;
        }
    }

    // 스택 실패 시 스왑/이동 시도
    bool bResult = false;
    if (SourceBackpackWidget)
    {
        // 가방 슬롯 간 스왑
        bResult = ToolbarInventory->TrySwapBackpackSlots(SourceBackpackWidget->BackpackSlotIndex, this->BackpackSlotIndex);
        LOG_Item_WARNING(TEXT("[TryStackOrSwap] 가방 스왑 결과: %s"), bResult ? TEXT("성공") : TEXT("실패"));
    }
    else
    {
        // 툴바에서 가방으로 이동
        bResult = ToolbarInventory->TryMoveToolbarItemToBackpack(SourceWidget->SlotIndex, this->BackpackSlotIndex);
        LOG_Item_WARNING(TEXT("[TryStackOrSwap] 툴바→가방 이동 결과: %s"), bResult ? TEXT("성공") : TEXT("실패"));
    }

    return bResult;
}

bool UBackpackSlotWidget::CanStack(const FBaseItemSlotData& SourceItem, const FBaseItemSlotData& TargetItem) const
{
    // 소스가 비어있으면 스택 불가
    if (UInventoryUtility::IsDefaultItem(SourceItem.ItemRowName) || SourceItem.Quantity <= 0)
    {
        return false;
    }

    // 대상이 비어있으면 스택 불가 (일반 이동으로 처리)
    if (UInventoryUtility::IsDefaultItem(TargetItem.ItemRowName) || TargetItem.Quantity <= 0)
    {
        return false;
    }

    // 동일한 아이템이 아니면 스택 불가
    if (SourceItem.ItemRowName != TargetItem.ItemRowName)
    {
        return false;
    }

    if (!ItemDataTable)
    {
        LOG_Item_WARNING(TEXT("[CanStack] ItemDataTable이 null"));
        return false;
    }

    const FItemDataRow* ItemRowData = ItemDataTable->FindRow<FItemDataRow>(TargetItem.ItemRowName, TEXT("CanStack"));
    if (!ItemRowData)
    {
        LOG_Item_WARNING(TEXT("[CanStack] ItemData를 찾을 수 없음: %s"), *TargetItem.ItemRowName.ToString());
        return false;
    }

    return TargetItem.Quantity < ItemRowData->MaxStack;
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