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

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void OnRep_ReplicatedHasBegunPlay() override;
	virtual void PostInitializeComponents() override;

public:
	/** 현재 생존 중인 플레이어 수 */
	UPROPERTY(ReplicatedUsing = OnRep_AlivePlayerCount, BlueprintReadOnly)
	int32 AlivePlayerCount;

	/** 생존자 수 갱신용 함수 */
	void AddAlivePlayer();
	void RemoveAlivePlayer();

	UFUNCTION()
	void OnRep_AlivePlayerCount();

	void MarkPlayerAsEscaped(APlayerState* EscapedPlayer);
	void CheckGameEndCondition();

	UFUNCTION(Server, Reliable)
	void Server_MarkPlayerAsEscaped(APlayerController* Controller);
	void Server_MarkPlayerAsEscaped_Implementation(APlayerController* Controller);

};