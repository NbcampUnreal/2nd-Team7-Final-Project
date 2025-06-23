#pragma once

#include "CoreMinimal.h"
#include "DataTable/MapDataRow.h"
#include "GameFramework/GameStateBase.h"
#include "LCGameState.generated.h"

UCLASS()
class LASTCANARY_API ALCGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	ALCGameState();

	virtual void PostInitializeComponents() override;

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_ReplicatedHasBegunPlay() override;

public:
	virtual void BeginPlay();
	virtual void InitGameState();
	void OnGameStart();
	void OnGameEnd();

	int32 TotalPlayerCount;

	int32 DeathPlayerCount;

	int32 EscapedPlayerCount;

	/** 현재 생존 중인 플레이어 수 */
	UPROPERTY(ReplicatedUsing = OnRep_AlivePlayerCount, BlueprintReadOnly)
	int32 AlivePlayerCount;

	/** 생존자 수 갱신용 함수 */
	void AddAlivePlayer();
	void RemoveAlivePlayer();

	UFUNCTION()
	void OnRep_AlivePlayerCount();

	void OnPlayerDeath(APlayerState* DeadPlayer);

	void MarkPlayerAsEscaped(APlayerState* EscapedPlayer);
	void CheckGameEndCondition();
};