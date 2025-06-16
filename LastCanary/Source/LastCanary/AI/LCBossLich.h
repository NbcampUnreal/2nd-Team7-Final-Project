#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "Particles/ParticleSystem.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "LCBossLich.generated.h"

// 전방 선언
class AArcaneBolt;

UCLASS()
class LASTCANARY_API ALCBossLich : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossLich();

protected:
    // ── 라이프사이클 ───────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void UpdateRage(float DeltaSeconds) override;
    virtual void OnRep_IsBerserk() override;
    virtual bool RequestAttack(float TargetDistance) override;

	// ── 광폭화 ────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    float BerserkCooldownFactor = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    float BerserkDamageFactor = 1.3f;

    // 광폭화 시 재생할 이펙트 & 사운드
    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    UNiagaraSystem* BerserkEffectFX;

    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    USoundBase* BerserkSound;

    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    // ── Sound/Effect FX ───────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Effects")
    UParticleSystem* ArcaneBoltFX;

    UPROPERTY(EditAnywhere, Category = "Lich|Effects")
    UParticleSystem* SoulBindFX;

    UPROPERTY(EditAnywhere, Category = "Lich|Effects")
    UParticleSystem* SoulAbsorbFX;

    UPROPERTY(EditAnywhere, Category = "Lich|Sound")
    USoundBase* ArcaneBoltSound;

    UPROPERTY(EditAnywhere, Category = "Lich|Sound")
    USoundBase* SoulBindSound;

    UPROPERTY(EditAnywhere, Category = "Lich|Sound")
    USoundBase* SoulAbsorbSound;

    /** Arcane Bolt Projectile 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "Lich|Combat")
    TSubclassOf<AArcaneBolt> ArcaneBoltClass;

    // ── Arcane Bolt 속도 ───────────────────────────
    UPROPERTY(EditDefaultsOnly, Category = "Lich|Combat")
    float ArcaneBoltSpeed = 1200.f;

    // ── 언데드 소환(희생의 의식) ───────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Undead")
    TArray<TSubclassOf<APawn>> UndeadClasses;

    UPROPERTY(EditAnywhere, Category = "Lich|Undead", meta = (ClampMin = "5.0"))
    float UndeadSpawnInterval = 20.f;

    /** 최대 소환 가능한 언데드 수 */
    UPROPERTY(EditAnywhere, Category = "Lich|Undead", meta = (ClampMin = "1"))
    int32 MaxUndeadMinions = 5;

    /** 살아있는 언데드 한 마리당 초당 Rage 증가량 */
    UPROPERTY(EditAnywhere, Category = "Lich|Undead")
    float UndeadRagePerSecond = 2.f;

    /** 언데드 사망 시 감소량 (언데드 1마리당) */
    UPROPERTY(EditAnywhere, Category = "Lich|Undead")
    float UndeadDeathRagePenalty = 10.f;

    // 소환된 언데드 추적용
    UPROPERTY()
    TArray<APawn*> SpawnedUndeadMinions;

    FTimerHandle UndeadSpawnTimerHandle;
    void SpawnUndeadMinion();

    UFUNCTION()
    void OnUndeadDestroyed(AActor* DestroyedActor);

    // ── Rage ─────────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Rage")
    float MaxRage = 100.f;

    void AddRage(float Amount);

    // ── 마나 파동 ─────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseInterval = 25.f;

    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseDamage = 15.f;

    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseRadius = 600.f;

    FTimerHandle ManaPulseTimerHandle;
    void ExecuteManaPulse();

    // ── 공격 패턴 ─────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float ArcaneBoltCooldown = 4.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float ArcaneBoltRange = 2000.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulBindCooldown = 12.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulBindRange = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaThreshold = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaDamage = 50.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaRadius = 800.f;

    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float SoulAbsorbDamage = 20.f;

    float LastArcaneBoltTime = -FLT_MAX;
    float LastSoulBindTime = -FLT_MAX;
    bool  bHasUsedDeathNova = false;

    /** 공격 함수 */
    void ExecuteArcaneBolt(AActor* Target);
    void ExecuteSoulBind(AActor* Target);
    void ExecuteDeathNova();
    void ExecuteSoulAbsorb(AActor* Target);
	
};
