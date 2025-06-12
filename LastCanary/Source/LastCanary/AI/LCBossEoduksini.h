#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "Components/SphereComponent.h"
#include "LCBossEoduksini.generated.h"

UCLASS()
class LASTCANARY_API ALCBossEoduksini : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

    friend class ALCBaseBossAIController;

public:
    ALCBossEoduksini();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** 어둠 범위를 나타내는 SphereComponent */
    UPROPERTY(VisibleAnywhere, Category = "Darkness")
    USphereComponent* DarknessSphere;

    /** 플레이어가 Sphere에 들어왔을 때 호출 */
    UFUNCTION()
    void OnDarknessSphereBeginOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    /** 플레이어가 Sphere에서 나갔을 때 호출 */
    UFUNCTION()
    void OnDarknessSphereEndOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    /** ── 플레이어 시선 체크 ── */
    bool IsLookedAtByAnyPlayer() const;
    bool IsPlayerLooking(APlayerController* PC) const;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Sight", meta = (ClampMin = "0", ClampMax = "180"))
    float LookAngleDeg = 18.f;

    /** ── 분노(Rage) ── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float MaxRage = 100.f;

    /** 어둠이 적용될 최대 반경 */
    UPROPERTY(EditAnywhere, Category = "Darkness")
    float DarknessRadius = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float DarknessRage = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageGainPerSec = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageLossPerSec = 12.f;

    /** ── 스케일(scale) ── */
    UPROPERTY(ReplicatedUsing = OnRep_CurScale, BlueprintReadOnly, Category = "Boss|Scale")
    float CurScale = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float MinScale = 0.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float MaxScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float ScaleInterpSpeed = 1.5f;

    UFUNCTION()
    void OnRep_CurScale();

    /** 페이드 강도, 시간 (디테일에서 조정 가능) */
    UPROPERTY(EditAnywhere, Category = "Darkness")
    float DarknessFadeAlpha = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Darkness")
    float FadeDuration = 0.5f;

    /** ── Darkness 스킬 ── */
    UPROPERTY(ReplicatedUsing = OnRep_DarknessActive, BlueprintReadOnly, Category = "Boss|Darkness")
    bool bDarknessActive = false;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Darkness")
    float DarknessDuration = 10.f;

    /** ── 광폭화 모드(Berserk) 관련 ── */
    
    /** Berserk 상태에서 Rage 상승 속도를 몇 배로 할지 */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float BerserkRageGainMultiplier = 2.0f;

    /** Berserk 상태에서 StrongAttackChance를 얼마나 올릴지 (예: 0.3 → 0.6) */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float StrongAttackChance_Berserk = 0.6f;

    /** Berserk 상태에서 NormalAttackCooldown을 얼마나 줄일지 (초 단위) */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float NormalAttackCooldown_Berserk = 0.6f;

    /** Berserk 상태에서 StrongAttackCooldown을 얼마나 줄일지 (초 단위) */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float StrongAttackCooldown_Berserk = 3.0f;

    /** Berserk 상태에서 몽타주 재생 속도 배수 */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float BerserkPlayRateMultiplier = 1.3f;

    UFUNCTION()
    void OnRep_DarknessActive();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartDarkness();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EndDarkness();

    UFUNCTION(BlueprintNativeEvent, Category = "Darkness")
    void BP_StartDarknessEffect();
    virtual void BP_StartDarknessEffect_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Darkness")
    void BP_EndDarknessEffect();
    virtual void BP_EndDarknessEffect_Implementation();

    /** Darkness 관련 타이머 */
    FTimerHandle DarknessTimer;

    UPROPERTY()
    TSet<APlayerController*> DarkenedPlayers;

    /** ── 공격(Rage 증감 포함) ── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float RageGain_Normal = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float RageLoss_Strong = 40.f;

    virtual bool RequestAttack() override;

private:
    void UpdateRageAndScale(float DeltaSeconds);
    void TryTriggerDarkness();
    void EndDarkness();

    void PlayNormalWithRage();
    void PlayStrongWithRage();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
