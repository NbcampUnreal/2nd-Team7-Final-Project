#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "LCBossMonsterCharacter.generated.h"

class UNiagaraSystem;
class UTimelineComponent;

UCLASS()
class LASTCANARY_API ALCBossMonsterCharacter : public ABaseMonsterCharacter
{
	GENERATED_BODY()

	friend class ALCBossAIController;

public:
    ALCBossMonsterCharacter();

    /* ───── 설정값(EditDefaultsOnly) ───── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Sight", meta = (ClampMin = "0.0", ClampMax = "180.0"))
    float LookAngleDeg = 18.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float MaxRage = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float DarknessRage = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageGainPerSec = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageLossPerSec = 12.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float MinScale = 0.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float MaxScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float ScaleInterpSpeed = 1.5f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Darkness")
    float DarknessDuration = 10.f;

    /* ─── 공격 확률·쿨타임 ─── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StrongAttackChance = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float NormalAttackCooldown = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float StrongAttackCooldown = 6.0f;

    /** 공격 몽타주 */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    UAnimMontage* NormalAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    UAnimMontage* StrongAttackMontage;

    /** 공격 시 분노 증감량 */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float RageGain_Normal = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float RageLoss_Strong = 40.f;

    /**
     * BTTask에서 호출하는 단일 공격 진입 함수
     * 성공 시 true, 쿨다운 등으로 실패 시 false
     */
    UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
    bool RequestAttack();

    /* ───── 실시간 복제 상태 ───── */
    UPROPERTY(ReplicatedUsing = OnRep_Rage, BlueprintReadOnly, Category = "Boss|Rage")
    float Rage = 0.f;

    UPROPERTY(ReplicatedUsing = OnRep_Scale, BlueprintReadOnly, Category = "Boss|Scale")
    float CurScale = 1.f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss|Darkness")
    bool bDarknessActive = false;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** 플레이어 시선 체크 */
    bool IsLookedAtByAnyPlayer() const;
    bool IsPlayerLooking(class APlayerController* PC) const;

    /** 분노·스케일 업데이트 */
    void UpdateRage(float DeltaSeconds, bool bLooked);
    void UpdateScale(float DeltaSeconds, bool bLooked);

    /** Darkness 스킬 */
    void TryTriggerDarkness();
    void EndDarkness();

    /** RepNotify 핸들러 */
    UFUNCTION()
    void OnRep_Rage();

    UFUNCTION()
    void OnRep_Scale();

    /** 네트워크 멀티캐스트 */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartDarkness();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EndDarkness();

    /** BP 후처리 훅 */
    UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Darkness")
    void BP_StartDarknessEffect();

    UFUNCTION(BlueprintImplementableEvent, Category = "Boss|Darkness")
    void BP_EndDarknessEffect();

    /** Darkness 타이머 */
    FTimerHandle DarknessTimer;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    float LastNormalTime = -FLT_MAX;
    float LastStrongTime = -FLT_MAX;

    void PlayNormal();
    void PlayStrong();
	
};
