#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "BaseItemSlotData.generated.h"

USTRUCT(BlueprintType)
struct FBaseItemSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemRowName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Durability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsEquipped;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsValid;

    FBaseItemSlotData()
        : ItemRowName(NAME_None)
        , Quantity(1)
        , Durability(100)
        , bIsEquipped(false)
        , bIsValid(false)
    {
    }
};