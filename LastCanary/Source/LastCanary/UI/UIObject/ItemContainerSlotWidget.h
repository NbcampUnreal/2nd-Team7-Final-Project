// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventorySlotWidget.h"
#include "ItemContainerSlotWidget.generated.h"

class UItemContainerWidget;

UCLASS()
class LASTCANARY_API UItemContainerSlotWidget : public UInventorySlotWidget
{
	GENERATED_BODY()
	
public:
    /** 컨테이너 내 슬롯 인덱스 */
    UPROPERTY(BlueprintReadOnly, Category = "Container")
    int32 ContainerSlotIndex = -1;

    /** 부모 컨테이너 위젯 참조 */
    UPROPERTY(BlueprintReadOnly, Category = "Container")
    UItemContainerWidget* ContainerWidget;

protected:
    /** 드롭 처리 오버라이드 - 컨테이너 전용 로직 */
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    /** 드래그 시작 처리 오버라이드 */
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;

private:
    /** 드래그 드롭 처리 (스택 또는 이동) */
    bool HandleDragDropOperation(UInventorySlotWidget* SourceWidget);

    /** 컨테이너 슬롯 간 아이템 교환 */
    bool HandleContainerToContainerDrop(UItemContainerSlotWidget* SourceWidget);

    /** 플레이어 인벤토리에서 컨테이너로 아이템 이동 */
    bool HandlePlayerToContainerDrop(UInventorySlotWidget* SourceWidget);

    /** 빈 슬롯에 아이템 배치 가능한지 확인 */
    bool CanPlaceItemInEmptySlot(const FBaseItemSlotData& ItemData) const;

    /** 동일 아이템 스택 가능한지 확인 */
    bool CanStackWithItem(const FBaseItemSlotData& SourceItem) const;
};
