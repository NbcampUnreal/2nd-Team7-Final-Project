#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCGimmickSpawnTypes.h"
#include "LCGimmickSpawner.generated.h"

UCLASS()
class LASTCANARY_API ALCGimmickSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	ALCGimmickSpawner();

protected:
	virtual void BeginPlay() override;

public:
	/** 세트 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	int32 GroupId;

	/** 역할 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	ESpawnerRole GimmickRole;

	/** 스폰 후보 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TArray<FSpawnCandidate> Candidates;

	/** 레벨에 배치된 기믹 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner")
	TSoftObjectPtr<AActor> ExternalLinkedActor;

	/** 스폰 시 초기 배치 옵션 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawner|Spawn")
	bool bUseSpawnerTransform = true;

	/** 스폰된 액터 */
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category = "Spawner")
	TObjectPtr<AActor> SpawnedActor;

	/** 스폰대상 선택 */
	UFUNCTION(BlueprintCallable, Category = "Spawner")
	TSubclassOf<AActor> PickClassByWeight() const;
};
