#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "LCBossLich.generated.h"

UCLASS()
class LASTCANARY_API ALCBossLich : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossLich();

protected:
    // ─── 기본 라이프사이클 ─────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void UpdateRage(float DeltaSeconds) override;
    virtual void OnRep_IsBerserk() override;
    virtual bool RequestAttack(float TargetDistance) override;

    // ─── 언데드 소환(희생의 의식) ─────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Undead")
    TArray<TSubclassOf<APawn>> UndeadClasses;

    UPROPERTY(EditAnywhere, Category = "Lich|Undead", meta = (ClampMin = "5.0"))
    float UndeadSpawnInterval = 20.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Undead")
    float UndeadRagePerSecond = 2.f;  // 살아있는 언데드 한 마리당 초당 Rage

    FTimerHandle UndeadSpawnTimerHandle;
    void SpawnUndeadMinion();

    // ─── Rage ─────────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Rage")
    float MaxRage = 100.f;

    void AddRage(float Amount);

    // ─── 마나 파동 ─────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseInterval = 25.f;

    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseDamage = 15.f;

    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseRadius = 600.f;

    FTimerHandle ManaPulseTimerHandle;
    void ExecuteManaPulse();

    // ─── 공격 패턴 ─────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float ArcaneBoltCooldown = 4.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float ArcaneBoltRange = 2000.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulBindCooldown = 12.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulBindRange = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaThreshold = 0.5f;   // 체력 비율

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaDamage = 50.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaRadius = 800.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulAbsorbDamage = 20.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulAbsorbHeal = 30.f;

    float LastArcaneBoltTime = -FLT_MAX;
    float LastSoulBindTime = -FLT_MAX;
    bool  bHasUsedDeathNova = false;

    void ExecuteArcaneBolt(AActor* Target);
    void ExecuteSoulBind(AActor* Target);
    void ExecuteDeathNova();
    void ExecuteSoulAbsorb(AActor* Target);

    // ─── Phylactery 기믹 ─────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Phylactery")
    AActor* PhylacteryActor = nullptr;

    void CheckPhylactery();

	
};
