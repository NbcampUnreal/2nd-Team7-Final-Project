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

protected:
    virtual void BeginPlay() override;
    virtual bool RequestAttack(float TargetDistance) override;

    /** 광폭화 */
    UPROPERTY(EditAnywhere, Category = "Vampire|Bloodlust")
    UNiagaraSystem* BloodlustEffectFX;

    UPROPERTY(EditAnywhere, Category = "Vampire|Bloodlust")
    USoundBase* BloodlustSound;

    // 그리고 기존에 선언만 해 두셨던 타이머와 가상 함수들을 확인합니다.
    FTimerHandle BerserkTimerHandle;
    virtual void OnRep_IsBerserk() override;
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;


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
    void EndMistForm();

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

    // ── Helpers ──
    void AddRage(float Amount);

    // ── Replication ──
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
