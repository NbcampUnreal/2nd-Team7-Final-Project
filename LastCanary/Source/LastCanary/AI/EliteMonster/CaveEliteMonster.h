#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "CaveEliteMonster.generated.h"

UCLASS()
class LASTCANARY_API ACaveEliteMonster : public ABaseMonsterCharacter
{
	GENERATED_BODY()
	
public:
	ACaveEliteMonster();

	void FreezeAI();

	void UnfreezeAI();

	void CooldownEnd();

	virtual void HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaxAge")
	float HearingMaxAge = 6.5f;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastAIGimmick();
	void MulticastAIGimmick_Implementation();

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void PlayGimmickSound();

	UFUNCTION()
	void ForgetTarget();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Combat")
	float MaxFreezeTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Gimmick|Combat")
	float FreezeCooldown = 7.f;

	UPROPERTY(BlueprintReadOnly, Category = "Gimmick|Combat")
	bool bIsFrozen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* GimmickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AIGimmick;

	UPROPERTY(VisibleAnywhere, Replicated, BlueprintReadOnly, Category = "Gimmick|Stress")
	int32 FreezeCount = 0;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Stress")
	int32 MaxFreezeCountForStress = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Gimmick|Stress")
	float ExplosionNoiseRange = 5000.0f;

	void CheckForStressMode();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick|Stress")
	float StressImmunityTime = 10.0f;

	bool bIsStressImmune = false;

	FTimerHandle StressImmunityTimerHandle;

	void EndStressImmunity();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Stress")
	void BP_OnStressModeStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "AI|Stress")
	void BP_OnStressModeEnd();
private:
	FTimerHandle CooldownTimerHandle;
	FTimerHandle FreezeTimerHandle;
	FTimerHandle ForgetTargetTimerHandle;
};