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

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    UDataTable* ResourceItemTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    UDataTable* ResourceCategoryTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
    UDataTable* RankThresholdTable;
};
