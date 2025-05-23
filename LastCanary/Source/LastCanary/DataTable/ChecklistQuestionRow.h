#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ChecklistQuestionRow.generated.h"

/**
 * 데이터테이블(Row) 기반으로 사용할 질문 정보 구조체
 */

USTRUCT(BlueprintType)
struct FChecklistQuestionRow : public FTableRowBase
{
	GENERATED_BODY()

	// 질문의 고유 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FName QuestionID;
	// 로컬라이징 키 (예: "Checklist.Question.1")
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText QuestionText;
	// 정답 (True/False, 보스 기준)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	bool bCorrectAnswer;
};
