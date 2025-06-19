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

	FTimerHandle ForgetTargetTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaxAge")
	float HearingMaxAge = 6.5f;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	float MaxFreezeTime = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
	float FreezeCooldown = 7.f;

	UPROPERTY(BlueprintReadOnly, Category = "Gimmick")
	bool bIsFrozen = false;

	FTimerHandle FreezeTimerHandle;

	FTimerHandle CooldownTimerHandle;

	virtual void HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus) override;

	UFUNCTION()
	void ForgetTarget();

private:
	void CooldownEnd();
};