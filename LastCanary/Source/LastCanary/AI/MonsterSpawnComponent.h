#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "MonsterSpawnComponent.generated.h"

class ANavMeshBoundsVolume;
class ABaseMonsterCharacter;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class LASTCANARY_API UMonsterSpawnComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMonsterSpawnComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

    UFUNCTION(BlueprintCallable, Category = "Monster Spawner")
    void StartSpawning();

    UFUNCTION(BlueprintCallable, Category = "Monster Spawner")
    void StopSpawning();

    void CurrentMap();
protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner")
    TArray<TSubclassOf<ABaseMonsterCharacter>> MonsterClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UDataTable* MonsterDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "1"))
    int32 MaxMonsterCount = 4;

    /*UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "1"))
    float MonsterLifeTime = 100.0f;*/

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "0"))
    float MinSpawnRadius = 1200.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "0"))
    float MaxSpawnRadius = 1500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "0"))
    float SpawnCooldown = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner")
    bool bAutoStart = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner")
    bool bIsNighttime = false;

private:
    void SpawnMonsters();
    void SpawnNightMonsters();
    void DestroyAllMonsters();

    //NavMeshBoundsVolume
    FVector GetValidSpawnLocationInNavVolume(const FVector& OwnerLocation);

    UPROPERTY()
    TArray<ACharacter*> SpawnedMonsters;

    FTimerHandle SpawnTimerHandle;
    FTimerHandle DestroyTimerHandle;

    bool bIsSpawning;
    int32 ReSpawnCount;
};