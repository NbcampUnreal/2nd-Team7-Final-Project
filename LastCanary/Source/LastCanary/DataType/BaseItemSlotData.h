#pragma once

#include "CoreMinimal.h"
#include "Item/ItemBase.h"
#include "DataType/BackpackSlotData.h"
#include "BaseItemSlotData.generated.h"

USTRUCT(BlueprintType)
struct FBaseItemSlotData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ItemRowName = "Default";

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Quantity = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Durability = 100;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsEquipped = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsValid = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 FireMode = 0;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bWasAutoFiring = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsBackpack = false;

    // 가방 전용 데이터 (일반 아이템에는 무시됨)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (EditCondition = "bIsBackpack"))
    TArray<FBackpackSlotData> BackpackSlots;
};