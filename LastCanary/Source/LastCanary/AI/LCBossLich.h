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
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    float BerserkCooldownFactor = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    float BerserkDamageFactor = 1.3f;

    // 광폭화 시 재생할 이펙트 & 사운드
    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    UNiagaraSystem* BerserkEffectFX;

    UPROPERTY(EditAnywhere, Category = "Lich|Berserk")
    USoundBase* BerserkSound;

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

    // ── Phantom Vortex 특수기 ─────────────────────────
/** 팬텀 소용돌이 FX(나이아가라) */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities")
    UNiagaraSystem* PhantomVortexFX;

    /** 팬텀 소용돌이 사운드 */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities")
    USoundBase* PhantomVortexSound;

    /** 소용돌이 지속 시간 */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities", meta = (ClampMin = "0.1"))
    float PhantomVortexDuration = 5.0f;

    /** 소용돌이 반경 */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities", meta = (ClampMin = "0.0"))
    float PhantomVortexRadius = 600.0f;

    /** Tick 당 대미지량 */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities", meta = (ClampMin = "0.0"))
    float PhantomVortexDamagePerTick = 20.0f;

    /** Tick 간격 */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities", meta = (ClampMin = "0.1"))
    float PhantomVortexTickInterval = 1.0f;

    /** Rage 임계치(<= 이하) 도달 시 1회 사용 */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float PhantomVortexRageThreshold = 0.8f;  // RagePercent 기준

    /** 한 번만 쓰도록 체크 */
    bool bHasUsedPhantomVortex = false;

    /** 내부 타이머 핸들 */
    FTimerHandle PhantomVortexTickHandle;

    // 스킬 실행 함수
    void PhantomVortex();

    // Tick 마다 데미지 처리
    void TickPhantomVortexDamage();

    /** PhantomVortex 감속 계수 (예: 0.5 = 50% 속도로) */
    UPROPERTY(EditAnywhere, Category = "Lich|Abilities", meta = (ClampMin = "0.01", ClampMax = "1.0"))
    float PhantomVortexSlowMultiplier = 0.5f;

    // ── Rage ─────────────────────────────────────

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

    /** Death Nova 스턴 지속 시간 */
    UPROPERTY(EditAnywhere, Category = "Lich|Combat")
    float DeathNovaStunDuration = 2.f;

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
