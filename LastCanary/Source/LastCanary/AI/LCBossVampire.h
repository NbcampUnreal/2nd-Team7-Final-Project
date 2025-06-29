#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h" 
#include "Components/SphereComponent.h"
#include "Components/AudioComponent.h"
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

    // Attenuation asset (사운드 감쇠)
    UPROPERTY(EditAnywhere, Category = "Vampire|Sound")
    USoundAttenuation* AttackAttenuation;

    FTimerHandle BerserkTimerHandle;
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;
    virtual void UpdateBlackboardValues() override;

    UPROPERTY(EditAnywhere, Category = "Vampire|Berserk")
	float BerserkMistRadius = 1200.f;

    // ── Blood Drain (영혼 흡수 힐) ──
    /** 흡수 효율 (1.0 = 100% 피흡, 특수 상태 시 2.0 사용) */
    UPROPERTY(EditAnywhere, Category = "Vampire|BloodDrain")
    float BloodDrainEfficiency = 1.0f;

    // ── Bat Swarm ──
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat")
    TSubclassOf<AActor> BatSwarmClass;

    UPROPERTY()
    TArray<AActor*> SpawnedBatSwarm;

    // Bat Swarm 관련
    /** Bat 하나당 초당 Rage 증가량 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Swarm")
    float BatRagePerSecond = 0.00042f;

    // Bat 하나당 언데드처럼 죽었을 때 깎이는 Rage 양
    UPROPERTY(EditAnywhere, Category = "Vampire|Swarm", meta = (ClampMin = "0.0"))
    float BatDeathRagePenalty = 2.0f;

    UFUNCTION()
    void OnBatDestroyed(AActor* DestroyedActor);

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

    /** 각 플레이어에 붙여줄 AudioComponent 보관 (OverlapEnd 시 정리) */
    TMap<APawn*, UAudioComponent*> MistAudioMap;

    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistDuration = 5.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistCooldown = 20.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistRadius = 800.f;

    // Mist 범위 콜리전 컴포넌트
    UPROPERTY(VisibleAnywhere, Category = "Vampire|Mist")
    USphereComponent* MistSphere;

    // Overlap 콜백
    UFUNCTION()
    void OnMistOverlapBegin(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
        bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnMistOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

    FTimerHandle MistDurationHandle, MistResetHandle;

    void EnterMistForm();

    UPROPERTY(EditAnywhere, Category = "Vampire|Mist|Sound")
    USoundAttenuation* MistSoundAttenuation;

    // ●●● 멀티캐스트 RPC ●●●
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayMistEnterEffects(APawn* Pawn);

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayMistExitEffects(const FVector& Location, const FRotator& Rotation);

    void EndMistForm();

    /** MistForm 중 플레이어에게 추가로 주는 초당 Rage */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistPlayerBonusRagePerSecond = 2.5f;

    /** MistForm 기본 범위(구) */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float DefaultMistRadius = 800.f;

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
    USoundBase* GazeSound;

    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeRadius = 600.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeInterval = 15.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeDebuffDuration = 5.f;

    FTimerHandle GazeTimerHandle;

    void ExecuteNightmareGaze();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayNightmareGazeEffects();

    // ── Crimson Slash ──
    /** Crimson Slash */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    UNiagaraSystem* CrimsonSlashEffectFX;

    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    USoundBase* CrimsonSlashSound;

    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    float CrimsonSlashRadius = 500.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    float CrimsonSlashDamage = 10.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonSlash")
    float CrimsonSlashCooldown = 8.f;

    FTimerHandle CrimsonSlashHandle;

    void ExecuteCrimsonSlash();

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayCrimsonSlashEffects();

    // ── Crimson Chains ────────────────────────────────
    /** 사슬 FX */
    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    UNiagaraSystem* CrimsonChainsEffectFX;

    UPROPERTY(EditAnywhere, Category = "Vampire|CrimsonChains")
    USoundBase* CrimsonChainsSound;

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

    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayCrimsonChainsEffects();

    // ── Sanguine Rain ────────────────────────────────
    /** 비 FX */
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    UNiagaraSystem* SanguineRainEffectFX;

    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineRain")
    USoundBase* SanguineRainSound;

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

    UPROPERTY(EditAnywhere, Category = "Vampire|Abilities", meta = (ClampMin = "0.0", ClampMax = "100.0"))
    float SanguineRainRageThreshold = 80.f;

    /** 클라이언트용 FX/SFX 재생 RPC */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlaySanguineRainEffects();

    void TickRainDamage();

    void EndSanguineRain();

    // ── Sanguine Burst ──
    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    USoundBase* SanguineBurstSound;

    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    float SanguineBurstRadius = 400.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    float SanguineBurstDamage = 15.f;

    UPROPERTY(EditAnywhere, Category = "Vampire|SanguineBurst")
    float SanguineBurstCooldown = 12.f;

    FTimerHandle BurstHandle;

    void ExecuteSanguineBurst();

    /** 클라이언트용 FX/SFX 재생 RPC */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlaySanguineBurstEffects();

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
