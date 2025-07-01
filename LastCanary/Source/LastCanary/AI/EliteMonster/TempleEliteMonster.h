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

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Combat")
	float HitCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Combat")
	float GroggyCount = 100;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Combat")
	TMap<FName, float> BoneHitCountMultipliers;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* GimmickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AIGimmick;
};