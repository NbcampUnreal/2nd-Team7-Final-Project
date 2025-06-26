#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "LCBossGumiho.generated.h"

UCLASS()
class LASTCANARY_API ALCBossGumiho : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossGumiho();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void UpdateRage(float DeltaSeconds) override;

    // ── RequestAttack override ─────────────────────────
    virtual bool RequestAttack(float TargetDistance) override;

    // ── 광폭화 ────────────────────────────────
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    virtual void OnRep_IsBerserk() override;

    /** SFX,VFX */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    UNiagaraSystem* BerserkEffectFX;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    USoundBase* BerserkSound;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities")
    UNiagaraSystem* SpiritSpikeFX;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities")
    USoundBase* SpiritSpikeSound;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    UNiagaraSystem* FoxfireFX;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    USoundBase* FoxfireSound;

    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    UNiagaraSystem* TailStrikeFX;

    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    USoundBase* TailStrikeSound;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    UNiagaraSystem* CharmGazeFX;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    USoundBase* CharmGazeSound;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    UNiagaraSystem* IllusionSpawnFX;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    USoundBase* IllusionSpawnSound;

    /** 이동 속도 버프 배수 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    float BerserkSpeedMultiplier = 2.0f;

    /** 공격력 버프 배수 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    float BerserkDamageMultiplier = 1.5f;

    // ── Illusion Dance ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    TSubclassOf<AActor> IllusionClass;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumIllusions = 3;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionInterval = 20.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionRagePerSecond = 1.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionDeathPenalty = 10.f;

    FTimerHandle IllusionTimerHandle;
    TArray<AActor*> IllusionActors;
    void SpawnIllusions();
    UFUNCTION()
    void OnIllusionDestroyed(AActor* DestroyedActor);

    // ── Tail Strike ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    float TailStrikeRadius = 400.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    float TailStrikeDamage = 30.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    float TailStrikeCooldown = 10.f;

    FTimerHandle TailStrikeTimerHandle;
    void ExecuteTailStrike();

    // ── Spirit Spike 특수 공격 ─────────────────────────

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0"))
    float SpiritSpikeRadius = 500.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0"))
    float SpiritSpikeDamage = 80.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0"))
    float SpiritSpikeCooldown = 15.f;



    // 실제 실행 함수
    void ExecuteSpiritSpike(AActor* Target);

    // ── Foxfire Volley ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    TSubclassOf<AActor> FoxfireClass;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire", meta = (ClampMin = "1"))
    int32 FoxfireCount = 5;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    float FoxfireInterval = 15.f;

    FTimerHandle FoxfireTimerHandle;
    void ExecuteFoxfireVolley();

    // ── Illusion Swap ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionSwapInterval = 25.f;

    /** Swap할 플레이어를 찾을 반경 */
    UPROPERTY(EditAnywhere, Category = "Illusion|Swap")
    float IllusionSwapRadius = 800.f;

    FTimerHandle SwapTimerHandle;
    void PerformIllusionSwap();

    // ── Charm Gaze ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmInterval = 10.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmRagePerSecond = 5.f;

    FTimerHandle CharmTimerHandle;
    void ExecuteCharmGaze();

    // ── Nine-Tail Burst ──
    bool bHasUsedNineTail = false;
    void ExecuteNineTailBurst();

    // ── Divine Grace ──
    UPROPERTY(ReplicatedUsing = OnRep_DivineGrace)
    bool bIsDivineGrace = false;
    
    UFUNCTION()
    void OnRep_DivineGrace();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartDivineGrace();

    // ── Rage ──

    void AddRage(float Amount);

    // ── 스킬 쿨다운 추적용 타임스탬프 ───────────────────
    float LastFoxfireTime = -FLT_MAX;
    float LastTailStrikeTime = -FLT_MAX;
    float LastIllusionSwapTime = -FLT_MAX;
    float LastSpiritSpikeTime = -FLT_MAX;

    // ── Replication ──
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
};
