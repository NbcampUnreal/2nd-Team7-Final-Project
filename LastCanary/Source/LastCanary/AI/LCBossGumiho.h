#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h"
#include "Materials/MaterialInterface.h"
#include "AI/Summon/Illusion.h"
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
    virtual void UpdateBlackboardValues() override;

    // ── RequestAttack override ─────────────────────────
    virtual bool RequestAttack(float TargetDistance) override;

    // ── 광폭화 ────────────────────────────────
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    /** SFX,VFX */
    /** Tail Strike attenuation asset */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Sound")
    USoundAttenuation* AttackAttenuation;

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
    USoundBase* IllusionSpawnSound;

    /** Nine-Tail Burst sound */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Sound")
    USoundBase* NineTailBurstSound;

    /** 이동 속도 버프 배수 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    float BerserkSpeedMultiplier = 2.0f;

    /** 공격력 버프 배수 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    float BerserkDamageMultiplier = 1.5f;

    // ── Illusion Dance ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    TSubclassOf<AIllusion> IllusionClass;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumIllusions = 3;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionInterval = 20.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionRagePerSecond = 0.15f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionDeathPenalty = 2.f;

    FTimerHandle IllusionTimerHandle;
    UPROPERTY()
    TArray<AIllusion*> IllusionActors;
    void SpawnIllusions();
    UFUNCTION()
    void OnIllusionDestroyed(AActor* DestroyedActor);

    // ── Tail Strike ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    float TailStrikeRadius = 400.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    float TailStrikeDamage = 10.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|TailStrike")
    float TailStrikeCooldown = 8.f;

    FTimerHandle TailStrikeTimerHandle;
    void ExecuteTailStrike();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayTailStrikeEffects();

    // ── Spirit Spike 특수 공격 ─────────────────────────

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0"))
    float SpiritSpikeRadius = 500.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0"))
    float SpiritSpikeDamage = 15.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0"))
    float SpiritSpikeCooldown = 4.f;

    // 실제 실행 함수
    void ExecuteSpiritSpike(AActor* Target);

    /** 클라이언트에 VFX/SFX 재생용 RPC */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlaySpiritSpikeEffects(const FVector& Location);

    // ── Foxfire Volley ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    TSubclassOf<AActor> FoxfireClass;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire", meta = (ClampMin = "1"))
    int32 FoxfireCount = 5;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    float FoxfireInterval = 10.f;

    FTimerHandle FoxfireTimerHandle;
    void ExecuteFoxfireVolley();

    /** 클라이언트에 Foxfire FX/SFX 재생용 RPC */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayFoxfireVolleyEffects(const FVector& Origin);

    // ── Illusion Swap ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionSwapInterval = 10.f;

    /** Swap할 플레이어를 찾을 반경 */
    UPROPERTY(EditAnywhere, Category = "Illusion|Swap")
    float IllusionSwapRadius = 1000.f;

    FTimerHandle SwapTimerHandle;
    void PerformIllusionSwap();

    /** 클라이언트 이펙트/사운드 재생용 RPC */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayIllusionSwapEffects(const FVector& Origin);

    // ── Charm Gaze ──
    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmInterval = 10.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmRagePerSecond = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Swap")
    FName CharmTag = TEXT("Charmed");

    FTimerHandle CharmTimerHandle;

    void ExecuteCharmGaze();

    // ── Nine-Tail Burst ──
    bool bHasUsedNineTail = false;
    void ExecuteNineTailBurst();
    /** Rage 임계치(<= 이하) 도달 시 1회 사용 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Abilities", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float NineTailBurstRageThreshold = 80.f;

    /** Nine-Tail Burst 전 범위 데미지 값 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Combat")
    float NineTailBurstDamage = 40.f;

    /** Nine-Tail Burst post-process material */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Effects")
    UMaterialInterface* NineTailBurstPPMaterial;

    /** Blend weight for the post-process effect */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Effects", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float NineTailBurstPPWeight = 1.0f;

    /** Duration of the post-process effect in seconds */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Effects", meta = (ClampMin = "0.1"))
    float NineTailBurstPPDuration = 2.0f;

    /** RPC to trigger the post-process on all clients */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayNineTailBurstEffects();

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
