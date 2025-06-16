#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NavigationSystem.h"
#include "Materials/MaterialInterface.h"
#include "NiagaraSystem.h"
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

    // ── Fear Wave (공포 파동) ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearRadius = 1200.f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearInterval = 12.f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    UMaterialInterface* FearPostProcessMaterial;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float FearPPBlendWeight = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Fear")
    float FearPPDuration = 1.0f;

    FTimerHandle FearTimerHandle;
    void ExecuteFearWave();

    // ── Abyssal Whisper (심연의 속삭임) ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Whisper")
    float WhisperInterval = 20.f;

    FTimerHandle WhisperTimerHandle;
    void ExecuteAbyssalWhisper();

    // ── Teleport ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Teleport")
    float TeleportInterval = 15.f;

    FTimerHandle TeleportTimerHandle;
    void TeleportToRandomLocation();

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

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ShadowGraspDistance = 1000.f;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float ShadowGraspCooldown = 12.f;
    FTimerHandle ShadowGraspTimerHandle;
    void ExecuteShadowGrasp();

    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float DistortionRadius = 500.f;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Attack")
    float DistortionCooldown = 20.f;
    FTimerHandle AttackDistortionTimerHandle;
    void ExecuteAttackDistortion();

    // ── Berserk ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    UNiagaraSystem* BerserkEffectFX;
    UPROPERTY(EditAnywhere, Category = "Slenderman|Berserk")
    USoundBase* BerserkSound;
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

    FTimerHandle BerserkTimerHandle;
    virtual void OnRep_IsBerserk() override;
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    // ── Rage ──
    UPROPERTY(EditAnywhere, Category = "Slenderman|Rage")
    float MaxRage = 100.f;
    void AddRage(float Amount);

    // ── Helpers ──
    bool IsPlayerLookingAtMe(APawn* PlayerPawn) const;

    // ── Replication ──
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
