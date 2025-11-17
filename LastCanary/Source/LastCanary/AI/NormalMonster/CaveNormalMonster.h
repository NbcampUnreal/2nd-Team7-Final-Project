#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "CaveNormalMonster.generated.h"

UCLASS()
class LASTCANARY_API ACaveNormalMonster : public ABaseMonsterCharacter
{
	GENERATED_BODY()
	
public:
	ACaveNormalMonster();

	FTimerHandle ForgetTargetTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaxAge")
	float HearingMaxAge = 6.5f;

protected:
	virtual void HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus) override;

	UFUNCTION()
	void ForgetTarget();
};
