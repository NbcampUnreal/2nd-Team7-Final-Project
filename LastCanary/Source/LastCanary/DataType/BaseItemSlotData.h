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

    FBaseItemSlotData()
        : ItemRowName(NAME_None)
        , Quantity(1)
        , Durability(100)
    {
    }
};