#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "WarderBaseMonster.generated.h"

UCLASS()
class LASTCANARY_API AWarderBaseMonster : public ABaseMonsterCharacter
{
	GENERATED_BODY()

public:
	AWarderBaseMonster();

	FTimerHandle ForgetTargetTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MaxAge")
	float HearingMaxAge = 6.5f;

	UFUNCTION()
	void WarderGimmick();
protected:
	virtual void HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus) override;

	UFUNCTION()
	void ForgetTarget();
};