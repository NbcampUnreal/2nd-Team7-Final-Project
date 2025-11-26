#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Actor/Gimmick/LCGimmickSpawnTypes.h"
#include "LCGimmickSpawnManager.generated.h"

class ALCGimmickSpawner;

UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCGimmickSpawnManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ALCGimmickSpawnManager();

protected:
	virtual void BeginPlay() override;

public:
	/** 전역 클래스별 스폰 한도(레벨 전체) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn|Quota")
	TArray<FGlobalSpawnQuota> GlobalQuotas;

	/** 스폰 지연(초) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawn")
	float SpawnDelay;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnSpawned(AActor* Spawned, ALCGimmickSpawner* Spawner);
	void Multicast_OnSpawned_Implementation(AActor* Spawned, ALCGimmickSpawner* Spawner);

private:
	FTimerHandle SpawnTimer;

	/** 클래스별 누적 카운트 */
	TMap<TSubclassOf<AActor>, int32> SpawnedCount;

	/** 전역 한도 체크 */
	bool CanSpawnClass(TSubclassOf<AActor> Class);

	/** 전체 스폰 */
	void ExecuteSpawn();

	/** 링크 주입(역할/세트/외부 액터 처리) */
	static void LinkActors(ALCGimmickSpawner* Spawner, AActor* Spawned, const TMap<int32, TArray<ALCGimmickSpawner*>>& Groups);

	/** 인터페이스 호출 헬퍼 */
	static void TrySetLinkedTarget(AActor* OnActor, AActor* Target);
	static void TrySetLinkedTrigger(AActor* OnActor, AActor* Trigger);

};
