#pragma once

#include "CoreMinimal.h"
#include "ChecklistQuestion.generated.h"

/**
 * 플레이어가 제출하는 응답 저장용 구조체
 */
USTRUCT(BlueprintType)
struct FChecklistQuestion
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly)
    FText QuestionText;

    UPROPERTY(BlueprintReadWrite)
    bool bAnswer = false;

    UPROPERTY(BlueprintReadWrite)
    bool bIsAnswered = false;
};