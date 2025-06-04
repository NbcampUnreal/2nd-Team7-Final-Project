// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NavigationInvokerComponent.h" 
#include "BaseBossMonsterCharacter.generated.h"

UCLASS()
class LASTCANARY_API ABaseBossMonsterCharacter : public ACharacter
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite, Category = Navigation, meta = (AllowPrivateAccess = "true"))
    UNavigationInvokerComponent* NavInvoker;

public:
    ABaseBossMonsterCharacter();

    /** ─── 공격 설정 ─── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    UAnimMontage* NormalAttackMontage;

    // ← 강공격 몽타주도 선언
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    UAnimMontage* StrongAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StrongAttackChance = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float NormalAttackCooldown = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float StrongAttackCooldown = 6.0f;

    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss|Attack")
    float Rage = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nav")
    float NavGenerationradius;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nav")
    float NavRemovalradius;

    /** 공격 진입 함수 (성공 시 true) */
    UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
    virtual bool RequestAttack();

protected:
    /** 실제 공격 처리 */
    virtual void PlayNormalAttack();
    virtual void PlayStrongAttack();

    /** 몽타주 종료 콜백 */
    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    float LastNormalTime = -FLT_MAX;
    float LastStrongTime = -FLT_MAX;

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
