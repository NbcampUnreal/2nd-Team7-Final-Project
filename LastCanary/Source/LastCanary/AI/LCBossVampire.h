#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h" 
#include "LCBossVampire.generated.h"

UCLASS()
class LASTCANARY_API ALCBossVampire : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossVampire();

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;
    virtual bool RequestAttack(float TargetDistance) override;

    /** 광폭화 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Bloodlust")
    UNiagaraSystem* BloodlustEffectFX;

    UPROPERTY(EditAnywhere, Category = "Vampire|Bloodlust")
    USoundBase* BloodlustSound;

    /** Spawn된 Berserk FX 컴포넌트 보관 */
    UPROPERTY()
    UNiagaraComponent* BerserkEffectComp = nullptr;

    /** Spawn된 Berserk Audio 컴포넌트 보관 */
    UPROPERTY()
    UAudioComponent* BerserkAudioComp = nullptr;

    FTimerHandle BerserkTimerHandle;
    virtual void OnRep_IsBerserk() override;
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    UPROPERTY(EditAnywhere, Category = "Vampire|Berserk")
	float BerserkMistRadius = 1200.f;

    // ── Blood Drain (영혼 흡수 힐) ──
    /** 흡수 효율 (1.0 = 100% 피흡, 특수 상태 시 2.0 사용) */
    UPROPERTY(EditAnywhere, Category = "Vampire|BloodDrain")
    float BloodDrainEfficiency = 1.0f;

    // ── Bat Swarm ──
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat")
    TSubclassOf<AActor> BatSwarmClass;
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat", meta = (ClampMin = "1"))
    int32 BatCount = 10;
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat")
    float BatSwarmInterval = 30.f;
    FTimerHandle BatSwarmTimerHandle;
    void ExecuteBatSwarm();

    // ── Mist Form ──
    // ── Mist Form FX/Sound ────────────────────────
    /** 무형 진입 시 재생할 Niagara 이펙트 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    UNiagaraSystem* MistEnterEffectFX;

    /** 무형 해제 시 재생할 Niagara 이펙트 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    UNiagaraSystem* MistExitEffectFX;

    /** 무형 진입 시 재생할 사운드 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    USoundBase* MistEnterSound;

    /** 무형 해제 시 재생할 사운드 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    USoundBase* MistExitSound;

    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistDuration = 5.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistCooldown = 20.f;
    bool bCanUseMist = true;
    UPROPERTY(ReplicatedUsing = OnRep_MistForm)
    bool bIsMistForm = false;
    FTimerHandle MistDurationHandle, MistResetHandle;
    UFUNCTION()
    void OnRep_MistForm();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartMistForm();
    void EnterMistForm();
    void TickMistFormRage();
    void EndMistForm();
    /** MistForm 중 기본 초당 Rage 회복량 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistRagePerSecond = 20.f;

    /** MistForm 중 플레이어에게 추가로 주는 초당 Rage */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistPlayerBonusRagePerSecond = 10.f;

    /** MistForm 기본 범위(구) */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float DefaultMistRadius = 800.f;

    /** MistForm 캐시 범위(구) */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistRadius = 800.f;

    /** 플레이어 카메라에 입힐 포스트프로세스 머티리얼 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    UMaterialInterface* MistPostProcessMaterial;

    /** 포스트프로세스 블렌드 가중치 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistPostProcessWeight = 0.5f;

    /** Mist Form 자동 발동 간격 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistFormInterval = 25.f;

    /** Mist Form 발동용 타이머 핸들 */
    FTimerHandle MistFormTimerHandle;
    FTimerHandle MistFormRageTimerHandle;

    // ── Nightmare Gaze ──
    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeRadius = 600.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeInterval = 15.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeDebuffDuration = 5.f;
    FTimerHandle GazeTimerHandle;
    void ExecuteNightmareGaze();

    // ── Crimson Slash ──
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    float CrimsonSlashRadius = 500.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    float CrimsonSlashDamage = 30.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    float CrimsonSlashCooldown = 8.f;
    FTimerHandle CrimsonSlashHandle;
    void ExecuteCrimsonSlash();

    // ── Crimson Chains ────────────────────────────────
    /** 사슬 FX */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    UNiagaraSystem* CrimsonChainsEffectFX;
    /** 범위(구형) */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    float CrimsonChainsRadius = 800.f;
    /** 끌어당기는 힘 */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    float CrimsonChainsPullStrength = 1200.f;
    /** 지속시간 */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    float CrimsonChainsDuration = 3.f;
    /** 재사용 대기시간 */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    float CrimsonChainsCooldown = 20.f;
    FTimerHandle CrimsonChainsTimerHandle;
    void ExecuteCrimsonChains();

    // ── Sanguine Rain ────────────────────────────────
    /** 비 FX */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    UNiagaraSystem* SanguineRainEffectFX;
    /** 영역 반경 */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    float RainRadius = 600.f;
    /** 비 지속시간 */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    float RainDuration = 8.f;
    /** 초당 출혈 데미지 */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    float RainDotDamage = 5.f;
    /** 이동 속도 감소 비율 */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    float RainSlowPercent = 0.5f;
    /** 피 웅덩이 클래스 */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    TSubclassOf<AActor> BloodPuddleClass;
    FTimerHandle RainTimerHandle, RainTickHandle;
    void ExecuteSanguineRain();
    void TickRainDamage();
    void EndSanguineRain();

    // ── Sanguine Burst ──
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    float SanguineBurstRadius = 400.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    float SanguineBurstDamage = 25.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    float SanguineBurstCooldown = 12.f;
    FTimerHandle BurstHandle;
    void ExecuteSanguineBurst();

    // ── Eternal Bloodlust (특수 상태) ──
    UPROPERTY(ReplicatedUsing = OnRep_Bloodlust)
    bool bIsBloodlust = false;
    UFUNCTION()
    void OnRep_Bloodlust();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartBloodlust();

    /** 특수 상태 지속시간 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Bloodlust")
    float BloodlustDuration = 20.f;
    FTimerHandle BloodlustDurationHandle;

    /** 캐시해둘 멤버 */
    float OriginalMoveSpeed;
    float OriginalCrimsonCooldown;
    float OriginalBurstCooldown;

    // ── Rage ──
    void AddRage(float Amount);

    virtual void UpdateRage(float DeltaSeconds) override;

    // ── Replication ──
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
