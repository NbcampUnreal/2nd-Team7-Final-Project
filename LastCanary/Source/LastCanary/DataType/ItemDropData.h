#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "ItemDropData.generated.h"

USTRUCT(BlueprintType)
struct FItemDropData
{
    GENERATED_BODY()

    // 스폰할 아이템 클래스 (예: ALootItem)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    TSubclassOf<AActor> ItemClass;

    // 몇 개 스폰할지 (1개 이상)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item")
    int32 Count = 1;
};
