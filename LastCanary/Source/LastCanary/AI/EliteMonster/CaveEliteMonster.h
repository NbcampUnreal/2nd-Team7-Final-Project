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

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	float MaxFreezeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	float FreezeCooldown = 7.f;

	UPROPERTY(BlueprintReadOnly, Category = "Gimmick")
	bool bIsFrozen = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool bIsInCooldown = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* GimmickSound;

	UPROPERTY(EditDefaultsOnly, Category = "Animation")
	UAnimMontage* AIGimmick;

private:
	FTimerHandle CooldownTimerHandle;
	FTimerHandle FreezeTimerHandle;
	FTimerHandle ForgetTargetTimerHandle;
};