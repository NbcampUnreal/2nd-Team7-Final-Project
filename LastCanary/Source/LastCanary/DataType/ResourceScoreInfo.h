#pragma once

#include "CoreMinimal.h"
#include "ResourceScoreInfo.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FResourceScoreInfo
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FName ResourceID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 BaseScore;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Multiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Amount;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 TotalScore;
};