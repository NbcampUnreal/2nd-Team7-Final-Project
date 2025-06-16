#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "LCGameManager.generated.h"


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
	void EndCurrentRound();
	int GetGold() const;
	void AddGold(int Amount);
};
