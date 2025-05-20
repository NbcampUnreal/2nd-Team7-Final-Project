#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameFramework/Character.h"
#include "MonsterSpawnComponent.generated.h"

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

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner")
    TArray<TSubclassOf<ACharacter>> MonsterClasses;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "1"))
    int32 MaxMonsterCount = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "1"))
    float MonsterLifeTime = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "0"))
    float MinSpawnRadius = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "0"))
    float MaxSpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner", meta = (ClampMin = "0"))
    float SpawnCooldown = 110.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner")
    bool bAutoStart = false;//½£¸Ê°¡¸é true·Î ÄÑ±â

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Monster Spawner")
    bool bIsNighttime = false;

private:
    void SpawnMonsters();

    void DestroyAllMonsters();

    UPROPERTY()
    TArray<ACharacter*> SpawnedMonsters;

    FTimerHandle SpawnTimerHandle;
    FTimerHandle DestroyTimerHandle;

    bool bIsSpawning;

    int32 ReSpawnCount;
};