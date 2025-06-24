#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/BaseItemSlotData.h"
#include "InventoryUtility.generated.h"

class UDataTable;
struct FItemDataRow;
class UInventoryConfig;

UCLASS()
class LASTCANARY_API UInventoryUtility : public UObject
{
	GENERATED_BODY()
	
public:
    /** 아이템 검증 관련 */
    UFUNCTION(BlueprintPure, Category = "Inventory Utility|Validation")
    static bool IsValidItem(FName ItemRowName, const UDataTable* ItemDataTable);

    UFUNCTION(BlueprintPure, Category = "Inventory Utility|Validation")
    static bool IsDefaultItem(FName ItemRowName, const UInventoryConfig* Config = nullptr);

    static bool IsBackpackItem(const FItemDataRow* ItemData);
    static bool IsCollectibleItem(const FItemDataRow* ItemData);

    UFUNCTION(BlueprintPure, Category = "Inventory Utility|Validation")
    static bool IsWalkieTalkieItem(FName ItemRowName, const UDataTable* ItemDataTable);

    /** 슬롯 관리 관련 */
    UFUNCTION(BlueprintCallable, Category = "Inventory Utility|Slot")
    static void SetSlotToDefault(FBaseItemSlotData& Slot, const UInventoryConfig* Config = nullptr);

    static bool CanStackItems(const FBaseItemSlotData& Slot, FName ItemRowName, const FItemDataRow* ItemData);

    UFUNCTION(BlueprintCallable, Category = "Inventory Utility|Slot")
    static int32 AddToStack(FBaseItemSlotData& Slot, int32 Amount, int32 MaxStack);

    /** 아이템 정보 관련 */
    UFUNCTION(BlueprintPure, Category = "Inventory Utility|Info")
    static float GetItemWeight(FName ItemRowName, const UDataTable* ItemDataTable);

    static FItemDataRow* GetItemDataByRowName(FName ItemRowName, const UDataTable* ItemDataTable);

    /** ID 변환 관련 */
    UFUNCTION(BlueprintPure, Category = "Inventory Utility|Conversion")
    static int32 GetItemIDFromRowName(FName ItemRowName, const UDataTable* ItemDataTable);

    UFUNCTION(BlueprintPure, Category = "Inventory Utility|Conversion")
    static FName GetItemRowNameFromID(int32 ItemID, const UDataTable* ItemDataTable);
};
