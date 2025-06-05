#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DataTable/ChecklistQuestionRow.h"
#include "DataType/ChecklistQuestion.h"
#include "DataType/GameResultData.h"
#include "ChecklistManager.generated.h"

USTRUCT(BlueprintType)
struct FChecklistResultData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	float CorrectRate = 0.f;

	UPROPERTY(BlueprintReadWrite)
	bool bIsSurvived = false;

	UPROPERTY(BlueprintReadWrite)
	int32 Score = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Rank = TEXT("C");

	UPROPERTY(BlueprintReadWrite)
	TArray<FResourceScoreInfo> ResourceDetails;

	UPROPERTY()
	APlayerController* OwnerController = nullptr;
};

class UChecklistWidget;
class UResultMenu;
class UResultEvaluator;
class APlayerController;
UCLASS()
class LASTCANARY_API AChecklistManager : public AActor
{
	GENERATED_BODY()

public:
	AChecklistManager();
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	void StartChecklist();

	int32 GetNumPlayers() const;

protected:
	UPROPERTY()
	TMap<APlayerController*, FChecklistResultData> PlayerResults;

	int32 SubmittedCount = 0;

	int32 TotalPlayerCount = 0;

	UPROPERTY()
	TArray<FChecklistQuestion> Questions;

	UPROPERTY()
	TArray<bool> CorrectAnswers;

	UPROPERTY(EditAnywhere, Category = "Checklist")
	UDataTable* ChecklistDataTable;

	UPROPERTY(EditAnywhere, Category = "Checklist")
	TSubclassOf<UChecklistWidget> ChecklistWidgetClass;

	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* ResourceItemTable;

	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* ResourceCategoryTable;

	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* RankThresholdTable;

	UPROPERTY(EditAnywhere, Category = "Checklist")
	TSubclassOf<UResultMenu> ResultMenuClass;

	UPROPERTY()
	UResultEvaluator* Evaluator;

public:
	UFUNCTION(BlueprintCallable)
	const TArray<bool>& GetCorrectAnswers() const { return CorrectAnswers; }

	void NotifyChecklistStartToAllPlayers();

	UFUNCTION(Server, Reliable)
	void Server_SubmitChecklist(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers);
	void Server_SubmitChecklist_Implementation(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers);

};
