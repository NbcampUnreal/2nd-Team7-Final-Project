#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "RankThresholdRow.generated.h"
/**
 * 
 */
USTRUCT(BlueprintType)
struct FRankThresholdRow : public FTableRowBase
{
    GENERATED_BODY()

    /** 랭크 문자열 (S / A / B / C 등) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Rank;

    /** 해당 랭크가 적용되는 최소 점수 (내림차순 정렬 권장) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 MinScore;
};
