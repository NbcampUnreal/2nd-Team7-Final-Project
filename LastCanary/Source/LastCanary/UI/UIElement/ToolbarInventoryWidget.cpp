// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "UI/UIObject/ToolbarSlotWidget.h"
#include "Components/HorizontalBox.h"
#include "LastCanary.h"

void UToolbarInventoryWidget::RefreshInventoryUI()
{
    UE_LOG(LogTemp, Warning, TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] 시작"));

    // 기본 유효성 검사
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

    // 기존 슬롯들 제거
    ToolbarSlotBox->ClearChildren();

    UToolbarInventoryComponent* ToolbarInventory = Cast<UToolbarInventoryComponent>(InventoryComponent);
    if (!ToolbarInventory)
    {
        LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] InventoryComponent 캐스팅 실패!"));
        return;
    }

    // 슬롯 위젯들 생성
    for (int32 i = 0; i < ToolbarInventory->GetMaxSlots(); ++i)
    {
        const FBaseItemSlotData& SlotData = ToolbarInventory->ItemSlots[i];

        // ⭐ 부모 클래스의 헬퍼 함수 사용
        UInventorySlotWidget* SlotWidget = CreateSlotWidget(i, SlotData);
        if (!SlotWidget)
        {
            continue;
        }

        // 툴바 전용 처리 (장착된 아이템 표시)
        if (UToolbarSlotWidget* ToolbarSlot = Cast<UToolbarSlotWidget>(SlotWidget))
        {
            if (i == ToolbarInventory->GetCurrentEquippedSlotIndex())
            {
                if (AItemBase* EquippedItem = ToolbarInventory->GetCurrentEquippedItem())
                {
                    ToolbarSlot->SetItemActor(EquippedItem);
                }
            }
        }

        // HorizontalBox에 추가
        ToolbarSlotBox->AddChild(SlotWidget);
    }

    LOG_Item_WARNING(TEXT("[ToolbarInventoryWidget::RefreshInventoryUI] UI 새로고침 완료 - 총 %d개 슬롯"),
        ToolbarInventory->GetMaxSlots());
}
