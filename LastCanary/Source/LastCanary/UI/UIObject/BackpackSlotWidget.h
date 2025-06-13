#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventorySlotWidget.h"
#include "BackpackSlotWidget.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UBackpackSlotWidget : public UInventorySlotWidget
{
	GENERATED_BODY()
	
public:
    /** 가방 내 슬롯 인덱스 (툴바 슬롯 인덱스와 구분) */
    UPROPERTY(BlueprintReadOnly, Category = "Backpack")
    int32 BackpackSlotIndex = -1;

protected:
    /** 드롭 처리 오버라이드 - 가방 전용 로직 */
    virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

    /** 드래그 시작 처리 오버라이드 */
    virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
};
