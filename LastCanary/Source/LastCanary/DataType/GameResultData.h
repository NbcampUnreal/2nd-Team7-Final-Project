#pragma once

#include "CoreMinimal.h"
#include "DataType/ResourceScoreInfo.h"
#include "GameResultData.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FGameResultData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    int32 CorrectChecklistCount;

    UPROPERTY(BlueprintReadOnly)
    int32 TotalChecklistCount;

    UPROPERTY(BlueprintReadOnly)
    int32 SurvivingPlayerCount;

    UPROPERTY(BlueprintReadOnly)
    int32 CollectedResourcePoints;

    UPROPERTY(BlueprintReadOnly)
    TArray<FResourceScoreInfo> ResourceScoreDetails;

    UPROPERTY(BlueprintReadOnly)
    int32 FinalScore;

    UPROPERTY(BlueprintReadOnly)
    FString Rank;
};