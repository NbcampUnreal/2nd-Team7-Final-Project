#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LCGameManager.generated.h"

USTRUCT(BlueprintType)
struct FResultInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FName, int32> PlayerResources;
};

USTRUCT(BlueprintType)
struct FGamePlayData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MapName;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int Round;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FString, FResultInfo> PlayerResult;
};


UCLASS()
class LASTCANARY_API ULCGameManager : public UGameInstanceSubsystem
{
	GENERATED_BODY()

protected:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	void InitGameManager();

public:
	int CurrentPlayerCount = 0; // 현재 플레이어 수
	int CurrentRound = 0; // 현재 라운드
	int MaxRounds = 3; // 최대 라운드 수

	int CurrentGold = 5000; // 현재 골드

	void StartGame();
	void InitCurrentRoundResult();
	void EndCurrentRound();
	void EndGame();

	bool IsGameEnd();

	void SubmitExplorationResults(FString PlayerName, bool bIsDeade, TMap<FName, int32> Results);

	int GetGold() const;
	void UpdateGold(FString Reason, int Amount);
	TArray<TPair<FString, int>> GoldHistory;
	void PrintGoldHistory();

	int GetPlayerCount() const { return CurrentPlayerCount; }

	FGamePlayData CurrentGamePlayData;
	TArray<FGamePlayData> AllGamePlayData; // 라운드 결과 정보

private:
	bool bIsGameStarted = false; // 게임 시작 여부

};
