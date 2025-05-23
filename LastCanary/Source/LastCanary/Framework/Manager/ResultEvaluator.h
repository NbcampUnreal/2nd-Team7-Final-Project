#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "DataType/ChecklistQuestion.h"
#include "DataType/GameResultData.h"
#include "ResultEvaluator.generated.h"

/**
 *
 */
UCLASS()
class LASTCANARY_API UResultEvaluator : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	FGameResultData EvaluateResult(const TArray<FChecklistQuestion>& PlayerAnswers,
		const TArray<bool>& CorrectAnswers,
		int32 SurvivingPlayers,
		int32 ResourcePoints);
};
