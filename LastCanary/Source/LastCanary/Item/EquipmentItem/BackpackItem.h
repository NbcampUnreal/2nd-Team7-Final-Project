#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "DataType/BackpackSlotData.h"
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
    /** 가방 내부 아이템 데이터 */
    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Backpack")
    TArray<FBackpackSlotData> BackpackData;

    /** 가방 최대 슬롯 수 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backpack Settings", meta = (ClampMin = "1", ClampMax = "50"))
    int32 BackpackSlots = 20;

    /** 가방 사용 (인벤토리 열기) */
    virtual void UseItem() override;

    /** 가방 데이터 가져오기 */
    UFUNCTION(BlueprintPure, Category = "Backpack")
    TArray<FBackpackSlotData> GetBackpackData() const;

    /** 가방에 데이터 설정 */
    UFUNCTION(BlueprintCallable, Category = "Backpack")
    void SetBackpackData(const TArray<FBackpackSlotData>& InData);

    /** 가방에 데이터 초기화 함수 */
    void InitializeBackpackSlots(TArray<FBackpackSlotData>& SlotsArray, int32 NumSlots);

    UFUNCTION(Client, Reliable)
    void Client_ToggleInventory();
    void Client_ToggleInventory_Implementation();
};
