#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "TempleEliteMonster.generated.h"

UCLASS()
class LASTCANARY_API ATempleEliteMonster : public ABaseMonsterCharacter
{
	GENERATED_BODY()
	
public:
	ATempleEliteMonster();

	FTimerHandle ForgetTargetTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaxAge")
	float HearingMaxAge = 6.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Attack")
	USphereComponent* Extra_AttackCollider;

	UFUNCTION(BlueprintCallable, Category = "Tracking")
	void ReceiveTrackingTarget(AActor* NewTarget);

	void ToggleOnReceive();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAIGimmick();
	void MulticastAIGimmick_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayGimmickSound();

protected:
	virtual void HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus) override;
	virtual void EnableAttackCollider() override;
	virtual void DisableAttackCollider() override;
	virtual void BeginPlay() override;

	UFUNCTION()
	void ForgetTarget();

	bool OnReceiveTracking = false;

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	float HitCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Combat")
	TMap<FName, float> BoneHitCountMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Combat")
	float GroggyCount = 100;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Combat")
	float EliteGroggy = 5.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Combat")//방어 기믹 끝나고 카운트 다시 시작, 연속 스턴 방어용
	float StunCooldownTime = EliteGroggy + 1.f;

	bool bIsInStunCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* GimmickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AIGimmick;

	UFUNCTION()
	void EndStunCooldown();

private:
	FTimerHandle StunCooldownTimerHandle;
};