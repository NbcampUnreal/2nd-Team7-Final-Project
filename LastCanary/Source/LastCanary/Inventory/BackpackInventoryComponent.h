#pragma once

#include "CoreMinimal.h"
#include "Inventory/InventoryComponentBase.h"
#include "DataType/BaseItemSlotData.h"
#include "BackpackInventoryComponent.generated.h"

/**
 * 배낭 인벤토리 컴포넌트 클래스
 * 플레이어의 기본 인벤토리 시스템을 관리합니다.
 */
UCLASS()
class LASTCANARY_API UBackpackInventoryComponent : public UInventoryComponentBase
{
    GENERATED_BODY()

public:
    /** 생성자 */
    UBackpackInventoryComponent();

protected:
    /** 컴포넌트 초기화 */
    virtual void BeginPlay() override;

    //-----------------------------------------------------
    // 인벤토리 내부 처리 (오버라이드)
    //-----------------------------------------------------

    /** 아이템을 인벤토리에 추가할 수 있는지 확인 */
    virtual bool CanAddItem(AItemBase* ItemActor) override;

    /** 아이템을 인벤토리에 저장 */
    virtual bool TryStoreItem(AItemBase* ItemActor) override;

    /** 아이템 추가 후 처리 */
    virtual void PostAddProcess() override;

public:
    //-----------------------------------------------------
    // 인벤토리 공개 인터페이스 (오버라이드)
    //-----------------------------------------------------

    /** 아이템을 인벤토리에 추가 (데이터 기반) */
    virtual bool TryAddItemSlot(FName ItemRowName, int32 Amount) override;

    /** 인벤토리에서 아이템 수량 감소 */
    virtual bool TryDecreaseItem(FName ItemRowName, int32 Amount) override;

    /** 인벤토리에서 특정 아이템의 수량 확인 */
    virtual int32 GetItemCount(FName ItemRowName) const override;

    /** 두 슬롯 간 아이템 교환 */
    virtual bool TrySwapItemSlots(int32 FromIndex, int32 ToIndex) override;

    /** 특정 슬롯의 아이템 제거 */
    virtual bool TryRemoveItemAtSlot(int32 SlotIndex) override;

    /** 실제 아이템 액터를 인벤토리에 추가 */
    virtual bool TryAddItem(AItemBase* ItemActor) override;
};