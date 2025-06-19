#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "Components/SphereComponent.h"
#include "Camera/CameraShakeBase.h"
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

    /** ── Darkness ── */
    UPROPERTY(VisibleAnywhere, Category = "Darkness")
    USphereComponent* DarknessSphere;

    UPROPERTY(EditAnywhere, Category = "Boss|Darkness")
    float DarknessRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Boss|Darkness")
    float DarknessFadeAlpha = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Boss|Darkness")
    float FadeDuration = 0.5f;

    UPROPERTY()
    TSet<APlayerController*> DarkenedPlayers;

    UFUNCTION()
    void OnDarknessSphereBeginOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex,
        bool bFromSweep,
        const FHitResult& SweepResult);

    UFUNCTION()
    void OnDarknessSphereEndOverlap(
        UPrimitiveComponent* OverlappedComp,
        AActor* OtherActor,
        UPrimitiveComponent* OtherComp,
        int32 OtherBodyIndex);

    /** ── Sight Check ── */
    bool IsLookedAtByAnyPlayer() const;
    bool IsPlayerLooking(APlayerController* PC) const;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Sight", meta = (ClampMin = "0", ClampMax = "180"))
    float LookAngleDeg = 18.f;    /** ── Rage ── */

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageGainPerSec = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageLossPerSec = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float DarknessRageThreshold = 60.f;

    /** ── Berserk Modifiers ── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float BerserkRageGainMultiplier = 2.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float StrongAttackChance_Berserk = 0.6f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float NormalAttackCooldown_Berserk = 0.6f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float StrongAttackCooldown_Berserk = 3.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float BerserkPlayRateMultiplier = 1.3f;

    /** ── Abilities ── */

    // Shadow Echo
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Ability")
    float ShadowEchoInterval = 15.f;
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Ability")
    float ShadowEchoDelay = 2.f;
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Ability")
	float ShadowEchoRange = 400.f; // 범위 내 모든 플레이어에게 데미지 적용
    float LastShadowEchoTime = -FLT_MAX;
    void ShadowEcho();
    void ExecuteShadowEchoDamage(FVector Location);

    // Nightmare Grasp
    float NightmareGraspInterval = 12.f;
    float LastNightmareGraspTime = -FLT_MAX;
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Ability")
	float NightmareGraspRange = 800.f; // 범위 내 모든 플레이어에게 견인
    void NightmareGrasp();

    // Night Terror
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Ability")
    float NightTerrorRageThreshold = 0.8f;
    bool bHasUsedNightTerror = false;
    void NightTerror();

    /** 공격확률 추가 */
    UPROPERTY(EditAnywhere, Category = "Boss|Ability")
    float PhaseShiftWeight = 1.f;
    UPROPERTY(EditAnywhere, Category = "Boss|Ability")
    float ShadowEchoWeight = 2.f;
    UPROPERTY(EditAnywhere, Category = "Boss|Ability")
    float NightmareGraspWeight = 3.f;
    UPROPERTY(EditAnywhere, Category = "Boss|Ability")
    float ShadowSwipeWeight = 5.f;
    UPROPERTY(EditAnywhere, Category = "Boss|Ability")
    float VoidGraspWeight = 4.f;



    /** ── Camera Shake ── */
    // 에디터에서 블루프린트 셰이크 클래스를 할당하세요
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Effects")
    TSubclassOf<UCameraShakeBase> TerrorCameraShakeClass;

	// ShadowSwipe
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float ShadowSwipeRange = 200.f; // 근접 공격 범위
    void ShadowSwipe();

	// Void Grasp
	UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
	float VoidGraspRange = 600.f; // 견인 범위
    void VoidGrasp();
    
    /** ── 공격 ── */
    virtual bool RequestAttack(float TargetDistance) override;

    /** ── Darkness State ── */
    UPROPERTY(ReplicatedUsing = OnRep_DarknessActive, BlueprintReadOnly, Category = "Boss|Darkness")
    bool bDarknessActive = false;

    UFUNCTION()
    void OnRep_DarknessActive();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartDarkness();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EndDarkness();

    UFUNCTION(BlueprintNativeEvent, Category = "Boss|Darkness")
    void BP_StartDarknessEffect();
    virtual void BP_StartDarknessEffect_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "Boss|Darkness")
    void BP_EndDarknessEffect();
    virtual void BP_EndDarknessEffect_Implementation();

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Darkness")
    float DarknessDuration = 10.f;
    

private:
    void UpdateRageAndScale(float DeltaSeconds);
    void TryTriggerDarkness();
    void EndDarkness();
    // Shadow Echo 폭발 딜레이용 핸들
    FTimerHandle ShadowEchoDamageHandle;
	// Darkness 종료 타이머 핸들
    FTimerHandle DarknessTimerHandle;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
