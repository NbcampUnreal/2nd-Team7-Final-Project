#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ChecklistQuestionRow.generated.h"

/**
 * 데이터테이블(Row) 기반으로 사용할 질문 정보 구조체
 * 보스마다 데이터테이블을 다르게 설정
 * 그 안에서 랜덤하게 질문을 N개 뽑는다.
 */

USTRUCT(BlueprintType)
struct FChecklistQuestionRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestionID;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText QuestionText;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCorrectAnswer;
};
