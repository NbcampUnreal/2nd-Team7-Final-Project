#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
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

    /** 면역·버프 연출용 이펙트 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    UNiagaraSystem* BerserkEffectFX;

    /** 현악기 선율 사운드 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Berserk")
    USoundBase* BerserkSound;

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

    // ── Replication ──
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	
};
