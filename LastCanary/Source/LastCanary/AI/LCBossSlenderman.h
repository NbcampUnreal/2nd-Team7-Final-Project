#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NavigationSystem.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
#include "Sound/SoundBase.h" 
#include "Character/BaseCharacter.h"
#include "LCBossSlenderman.generated.h"

UCLASS()
class LASTCANARY_API ALCBossSlenderman : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossSlenderman();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;
    virtual void UpdateRage(float DeltaSeconds) override;
    virtual bool RequestAttack(float TargetDistance) override;
    virtual void UpdateBlackboardValues() override;

    // VFX/SFX

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    UNiagaraSystem* ShadowGraspFX;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    USoundBase* ShadowGraspSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    UNiagaraSystem* EndlessStalkFX; 

    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    USoundBase* EndlessStalkSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    UNiagaraSystem* AttackDistortionFX;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    USoundBase* AttackDistortionSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Distortion")
    UNiagaraSystem* DistortionFX;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Distortion")
    USoundBase* DistortionSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Teleport")
    UNiagaraSystem* TeleportFX;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Teleport")
    USoundBase* TeleportSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Whisper")
    USoundBase* WhisperSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    UNiagaraSystem* ReachSlashFX;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    USoundBase* ReachSlashSound;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    UNiagaraSystem* FearWaveFX;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    USoundBase* FearWaveSound;

    /** SFX attenuation asset */
    UPROPERTY(EditAnywhere, Category = "Slenderman|ReachSlash")
    USoundAttenuation* AttackAttenuation;

    /** 기본 초당 Rage 회복량 (아무도 쳐다보지 않아도 이만큼은 회복) */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Rage", meta = (ClampMin = "0.0"))
    float BaseRagePerSecond = 0.35f; // 예: 0.35f/sec × 300초 ≈ 105 → 대략 5분 내 MaxRage 도달

    /** 플레이어가 바라보는 시야 허용 한계 (Dot>Threshold 일 때만 Rage 증가) */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Rage")
    float LookDotThreshold = 0.95f;

    /** 플레이어가 바라볼 때 초당 증가시킬 Rage 양 */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Rage")
    float LookRagePerSecond = 0.2f;

    /** 플레이어가 바라보지 않을 때 초당 감소시킬 Rage 양 */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Rage")
    float LoseRagePerSecond = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    UMaterialInterface* FearPostProcessMaterial;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearPPBlendWeight = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearPPDuration = 1.0f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearWaveDamage = 40.f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearSlowMultiplier = 0.6f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearSlowDuration = 4.f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearWaveRageThreshold = 80.f;

    FTimerHandle FearTimerHandle;
    void ExecuteFearWave();

    /** RPC: play VFX/SFX on all clients */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayFearWaveEffects();

    /** RPC: apply PP on this client */
    UFUNCTION(Client, Unreliable)
    void Client_ApplyFearPostProcess(ACharacter* Target);

    // ── Abyssal Whisper (심연의 속삭임) ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Whisper")
    float WhisperInterval = 60.f;

    FTimerHandle WhisperTimerHandle;
    void ExecuteAbyssalWhisper();

    /** Whisper 사운드를 모든 클라이언트에 재생하는 RPC */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayAbyssalWhisper(const FVector& Location);

    /** Whisper SFX attenuation asset */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Sound")
    USoundAttenuation* WhisperAttenuation;

    // ── Teleport ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Teleport")
    float TeleportInterval = 15.f;

    FTimerHandle TeleportTimerHandle;
    void TeleportToRandomLocation();

    /** RPC: play teleport effects on all clients */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayTeleportEffects(const FVector& Location);

    /** Attenuation asset for teleport sound */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Teleport")
    USoundAttenuation* TeleportAttenuation;

    // ── Distortion ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Distortion")
    float DistortionInterval = 30.f;

    FTimerHandle DistortionTimerHandle;
    void ExecuteDistortion();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_DistortionEffect();

    // ── Endless Stalk ──
    FTimerHandle EndlessStalkTimerHandle;

    UPROPERTY(ReplicatedUsing = OnRep_EndlessStalk)
    bool bIsEndlessStalk = false;

    /** 원래 이동 속도 저장 */
    float DefaultWalkSpeed = 0.f;

    UFUNCTION()
    void OnRep_EndlessStalk();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartEndlessStalk();
    void EnterEndlessStalk();

    // ── Attack Patterns ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ReachSlashRadius = 600.f;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ReachSlashDamage = 25.f;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ReachSlashCooldown = 8.f;
    FTimerHandle ReachSlashTimerHandle;
    void ExecuteReachSlash();

    /** 멀티캐스트 RPC: Reach Slash 효과 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayReachSlashEffects(const FVector& Origin);

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ShadowGraspDistance = 1000.f;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ShadowGraspCooldown = 12.f;
    FTimerHandle ShadowGraspTimerHandle;
    void ExecuteShadowGrasp();

    /** 멀티캐스트 RPC: Shadow Grasp 효과 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayShadowGraspEffects(const FVector& Location);

    /** Distortion 명령에 의해 함께 이동시킬 플레이어 캐릭터 목록 */
    UPROPERTY()
    TArray<ABaseCharacter*> AffectedPlayers;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float DistortionRadius = 500.f;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float DistortionCooldown = 20.f;
    /** Distortion 공격 후 텔레포트 거리 */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Distortion")
    float AttackDistortionRange = 800.f;
    FTimerHandle AttackDistortionTimerHandle;
    void ExecuteAttackDistortion();
    // --- RPC for client VFX/SFX ---
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayAttackDistortionEffects(const FVector& Location);

    // ── Berserk ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    float BerserkSpeedMultiplier = 1.5f;
    /** 분신 소환용 클래스(에디터에서 할당) */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    TSubclassOf<AActor> ShadowCloneClass;
    /** 소환된 분신 보관 */
    UPROPERTY()
    TArray<AActor*> ShadowClones;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    int32 BerserkCloneCount = 5;

    float LastReachSlashTime = -FLT_MAX;
    float LastShadowGraspTime = -FLT_MAX;
    float LastAttackDistortionTime = -FLT_MAX;

    FTimerHandle BerserkTimerHandle;
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    // ── Rage ──
    void AddRage(float Amount);

    // ── Helpers ──
    bool IsPlayerLookingAtMe(APawn* PlayerPawn) const;

    // ── Replication ──
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
