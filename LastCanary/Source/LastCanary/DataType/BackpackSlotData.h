#pragma once

#include "CoreMinimal.h"
#include "BackpackSlotData.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct LASTCANARY_API FBackpackSlotData
{
    GENERATED_BODY()

    /** 아이템 RowName (데이터테이블 참조용) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backpack")
    FName ItemRowName = "Default";

    /** 아이템 고유 ID (필요시) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backpack")
    FString ItemID = "";

    /** 아이템 수량 */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Backpack")
    int32 Quantity = 0;

    FBackpackSlotData()
    {
        ItemRowName = "Default";
        ItemID = "";
        Quantity = 0;
    }

    bool IsValid() const
    {
        return !ItemRowName.IsNone() && Quantity > 0;
    }

    bool IsEmpty() const
    {
        return ItemRowName.IsNone() || Quantity <= 0;
    }
};