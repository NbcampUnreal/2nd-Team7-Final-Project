#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "InventoryDropSystem.generated.h"

class UInventoryComponentBase;
class UToolbarInventoryComponent;
class AItemBase;
class UInventoryConfig;
struct FBaseItemSlotData;

UCLASS()
class LASTCANARY_API UInventoryDropSystem : public UObject
{
	GENERATED_BODY()
	
public:
    /** 통합된 아이템 드롭 실행 */
    UFUNCTION(BlueprintCallable, Category = "Inventory Drop System")
    static bool ExecuteDropItem(
        UInventoryComponentBase* Inventory,
        int32 SlotIndex,
        int32 Quantity,
        bool bIsEquipped = false
    );

    /** 아이템 이름으로 드롭 */
    UFUNCTION(BlueprintCallable, Category = "Inventory Drop System")
    static bool ExecuteDropItemByName(
        UInventoryComponentBase* Inventory,
        FName ItemRowName,
        int32 Quantity
    );

private:
    /** 드롭 요청 검증 */
    static bool ValidateDropRequest(UInventoryComponentBase* Inventory, int32 SlotIndex, int32 Quantity);

    /** 드롭 위치 계산 */
    static FVector CalculateDropLocation(AActor* Owner, const UInventoryConfig* Config = nullptr);

    /** 장착된 아이템 특별 처리 */
    static void HandleEquippedItemDrop(UToolbarInventoryComponent* Toolbar, int32 SlotIndex);

    /** 실제 드롭 실행 */
    static bool ExecuteDropLogic(
        UInventoryComponentBase* Inventory,
        int32 SlotIndex,
        int32 Quantity,
        const FBaseItemSlotData& DropItemData
    );
};
