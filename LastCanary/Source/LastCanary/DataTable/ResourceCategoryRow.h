#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataType/ResourceCategory.h"
#include "ResourceCategoryRow.generated.h"

/**
 * 계열별 점수 가중치
 */
USTRUCT(BlueprintType)
struct FResourceCategoryRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EResourceCategory Category;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ScoreMultiplier = 1.0f;
};
