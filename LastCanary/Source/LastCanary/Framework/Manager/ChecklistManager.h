#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataType/ChecklistQuestion.h"
#include "DataTable/ChecklistQuestionRow.h"
#include "ChecklistManager.generated.h"

class UChecklistWidget;
class UResultMenu;
class UResultEvaluator;
UCLASS()
class LASTCANARY_API AChecklistManager : public AActor
{
	GENERATED_BODY()

public:
	AChecklistManager();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void StartChecklist();

	UFUNCTION(BlueprintCallable)
	void OnChecklistSubmitted(const TArray<FChecklistQuestion>& PlayerAnswers);

protected:
	/** 질문 목록 (플레이어 입력 저장용) */
	UPROPERTY()
	TArray<FChecklistQuestion> Questions;

	/** 보스 기준 정답 목록 */
	UPROPERTY()
	TArray<bool> CorrectAnswers;

	/** 데이터테이블에서 질문 로딩 */
	UFUNCTION(BlueprintCallable)
	void InitializeQuestionsFromDataTable();

	/** 질문 원본 */
	UPROPERTY(EditAnywhere, Category = "Checklist")
	UDataTable* ChecklistDataTable;

	/** 질문 UI 위젯 */
	UPROPERTY(EditAnywhere, Category = "Checklist")
	TSubclassOf<UChecklistWidget> ChecklistWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* ResourceItemTable;

	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* ResourceCategoryTable;

	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* RankThresholdTable;

	/** 결과 UI 위젯 */
	UPROPERTY(EditAnywhere, Category = "Checklist")
	TSubclassOf<UResultMenu> ResultMenuClass;

	/** 평가 시스템 */
	UPROPERTY()
	UResultEvaluator* Evaluator;
};