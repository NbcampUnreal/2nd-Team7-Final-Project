#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/BaseItemSlotData.h"
#include "BackpackManager.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBackpackEquipped, const TArray<FBackpackSlotData>&, BackpackSlots);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBackpackUnequipped);

class UToolbarInventoryComponent;
class UDataTable;
class UInventoryConfig;

UCLASS()
class LASTCANARY_API UBackpackManager : public UObject
{
	GENERATED_BODY()
	
public:
    UBackpackManager();

    /** 매니저 초기화 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager")
    void Initialize(UToolbarInventoryComponent* InOwnerInventory);

    //-----------------------------------------------------
    // 가방 장착/해제
    //-----------------------------------------------------

    /** 가방 장착 처리 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|Equip")
    bool EquipBackpack(int32 ToolbarSlotIndex);

    /** 가방 해제 처리 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|Equip")
    bool UnequipBackpack(int32 ToolbarSlotIndex);

    /** 현재 장착된 가방이 있는지 확인 */
    UFUNCTION(BlueprintPure, Category = "Backpack Manager|Query")
    bool HasBackpackEquipped() const;

    /** 현재 가방 슬롯들 반환 */
    UFUNCTION(BlueprintPure, Category = "Backpack Manager|Query")
    TArray<FBackpackSlotData> GetCurrentBackpackSlots() const;

    //-----------------------------------------------------
    // 가방 아이템 관리
    //-----------------------------------------------------

    /** 가방에 아이템 추가 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|Items")
    bool AddItemToBackpack(FName ItemRowName, int32 Quantity, int32 BackpackSlotIndex = -1);

    /** 가방에서 아이템 제거 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|Items")
    bool RemoveItemFromBackpack(int32 BackpackSlotIndex, int32 Quantity);

    /** 가방 슬롯들 업데이트 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|Items")
    bool UpdateBackpackSlots(const TArray<FBackpackSlotData>& NewSlots);

    //-----------------------------------------------------
    // 드래그 & 드롭
    //-----------------------------------------------------

    /** 툴바 아이템을 가방으로 이동 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|DragDrop")
    bool MoveToolbarItemToBackpack(int32 ToolbarIndex, int32 BackpackIndex);

    /** 가방 아이템을 툴바로 이동 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|DragDrop")
    bool MoveBackpackItemToToolbar(int32 BackpackIndex, int32 ToolbarIndex);

    /** 가방 슬롯 간 아이템 교환 */
    UFUNCTION(BlueprintCallable, Category = "Backpack Manager|DragDrop")
    bool SwapBackpackSlots(int32 FromIndex, int32 ToIndex);

    //-----------------------------------------------------
    // 이벤트
    //-----------------------------------------------------

    UPROPERTY(BlueprintAssignable, Category = "Backpack Manager|Events")
    FOnBackpackEquipped OnBackpackEquipped;

    UPROPERTY(BlueprintAssignable, Category = "Backpack Manager|Events")
    FOnBackpackUnequipped OnBackpackUnequipped;

private:
    /** 소유자 인벤토리 참조 */
    UPROPERTY()
    UToolbarInventoryComponent* OwnerInventory;

    /** 현재 장착된 가방 슬롯 인덱스 */
    int32 CurrentBackpackSlotIndex;

    /** 빈 가방 슬롯 찾기 */
    int32 FindEmptyBackpackSlot(const TArray<FBackpackSlotData>& BackpackSlots) const;

    /** 가방 슬롯 검증 */
    bool ValidateBackpackSlot(int32 BackpackSlotIndex) const;

    /** 아이템이 가방에 넣을 수 있는지 확인 */
    bool CanAddToBackpack(FName ItemRowName) const;
};
