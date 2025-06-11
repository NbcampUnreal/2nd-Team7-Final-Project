#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "Inventory/BackpackInventoryComponent.h"
#include "BackpackItem.generated.h"

UCLASS()
class LASTCANARY_API ABackpackItem : public AEquipmentItemBase
{
    GENERATED_BODY()

public:
    ABackpackItem();

protected:
    virtual void BeginPlay() override;

public:
    /** 가방의 인벤토리 컴포넌트 (삭제 예정)*/
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory")
    UBackpackInventoryComponent* BackpackInventoryComponent;

    /** 가방 인벤토리 컴포넌트 접근자 (삭제 예정)*/
    UFUNCTION(BlueprintPure, Category = "Inventory")
    UBackpackInventoryComponent* GetBackpackInventoryComponent() const;

    /** 가방 내부 아이템 데이터 */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Backpack")
    TArray<FBaseItemSlotData> BackpackData;

    /** 가방 최대 슬롯 수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backpack Settings", meta = (ClampMin = "1", ClampMax = "50"))
    int32 BackpackSlots = 20;

    /** 가방 사용 (인벤토리 열기) */
    virtual void UseItem() override;

    /** 가방 데이터 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Backpack")
    TArray<FBaseItemSlotData> GetBackpackData() const;

    /** 가방에 데이터 설정 */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    void SetBackpackData(const TArray<FBaseItemSlotData>& NewData);

protected:
    /** 가방 초기화 */
    void InitializeBackpack();
};
