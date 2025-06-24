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

protected:
	virtual void HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus) override;

	virtual void EnableAttackCollider() override;
	virtual void DisableAttackCollider() override;

	UFUNCTION()
	void ForgetTarget();
};