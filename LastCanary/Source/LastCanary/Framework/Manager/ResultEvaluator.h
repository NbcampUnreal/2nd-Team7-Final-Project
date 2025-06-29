#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/ChecklistQuestion.h"
#include "DataType/GameResultData.h"
#include "DataType/ResourceCategory.h"
#include "ResultEvaluator.generated.h"

/**
 *
 */
class UDataTable;
UCLASS()
class LASTCANARY_API UResultEvaluator : public UObject
{
	GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable)
    FGameResultData EvaluateResult(
        const TArray<FChecklistQuestion>& PlayerAnswers,
        const TArray<bool>& CorrectAnswers,
        int32 SurvivingPlayers,
        const TMap<FName, int32>& CollectedResources);

    UFUNCTION(BlueprintCallable)
    FGameResultData EvaluatePlayerResult
    (
        const TArray<FChecklistQuestion>& PlayerAnswers,
        const TArray<bool>& CorrectAnswers,
        const bool bIsSurvive,
        const int32 SurviveTime,
        const int32 KillCount,
        const TMap<FName, int32>& CollectedResources,
        const TArray<int32> CollectedClues
    );

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    UDataTable* ResourceItemTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    UDataTable* ResourceCategoryTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    UDataTable* RankThresholdTable;

private:
    int32 SurvivePoint = 50;
};
