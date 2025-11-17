#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

USTRUCT(BlueprintType)
struct FPlayerResultData
{
	GENERATED_BODY()

public:
	UPROPERTY()
	APlayerController* OwnerController = nullptr;

	UPROPERTY()
	FString PlayerName = TEXT("C");

	UPROPERTY(BlueprintReadWrite)
	bool bIsSurvived = false;

	UPROPERTY(BlueprintReadWrite)
	float CorrectRate = 0.f;

	UPROPERTY(BlueprintReadWrite)
	int32 SurviveTime = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 KillCount = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FResourceScoreInfo> ResourceDetails;

	UPROPERTY(BlueprintReadWrite)
	int32 ResourcePoint = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 TotalScore = 0;

	UPROPERTY(BlueprintReadWrite)
	TArray<FExplorePointInfo> ExplorePointDetails;

	UPROPERTY(BlueprintReadWrite)
	int32 ExplorePoint = 0;

	UPROPERTY(BlueprintReadWrite)
	FString Rank = TEXT("C");
};

USTRUCT(BlueprintType)
struct FTotalResultData
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentRound = 0;

	UPROPERTY(BlueprintReadWrite)
	FString CurrentMap = "";

	UPROPERTY(BlueprintReadWrite)
	FString MVPName = "";

	UPROPERTY(BlueprintReadWrite)
	int32 TotalReources = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 Payment = 0;

	UPROPERTY(BlueprintReadWrite)
	int32 TotalEXP = 0;

	UPROPERTY(BlueprintReadWrite)
	FString TotalRank = TEXT("C");

	UPROPERTY(BlueprintReadWrite)
	TArray<FPlayerResultData> PlayersResult;
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

	void InitCheckListManager(UDataTable* CheckListTable);
	void StartChecklist();

	int32 GetNumPlayers() const;

protected:
	UPROPERTY()
	FTotalResultData TotalGameResult;
	UPROPERTY()
	TMap<APlayerController*, FPlayerResultData> NewPlayerResults;

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
	UPROPERTY(EditAnywhere, Category = "Checklist|Evaluation")
	UDataTable* TotalRankTable;

	UPROPERTY(EditAnywhere, Category = "Checklist")
	TSubclassOf<UResultMenu> ResultMenuClass;

	UPROPERTY()
	UResultEvaluator* Evaluator;

public:
	UFUNCTION(BlueprintCallable)
	const TArray<bool>& GetCorrectAnswers() const { return CorrectAnswers; }

	void NotifyChecklistStartToAllPlayers();

	void SubmitCheckList(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers);

	void StartResult();

	UFUNCTION(Server, Reliable)
	void Server_SubmitChecklist(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers);
	void Server_SubmitChecklist_Implementation(APlayerController* Submitter, const TArray<FChecklistQuestion>& PlayerAnswers);

	TMap<FName, int32> CollectAllPlayerResources();

	FString GetMVPName();
	int32 GetTotalResourcePoint();
	int32 GetTotalEXP();

private:
	void SetTotalGameResult();
	void AddOrUpdatePlayerResult(APlayerController* Submitter, const FPlayerResultData& PlayerResultData);
	bool IsAllPlayerSubmitCheckList();
};
