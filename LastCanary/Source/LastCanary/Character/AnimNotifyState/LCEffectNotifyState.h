#pragma once

#include "CoreMinimal.h"
#include "Character/AnimNotifyState/LCBaseNotifyState.h"
#include "NiagaraSystem.h"
#include "LCEffectNotifyState.generated.h"


UCLASS()
class LASTCANARY_API ULCEffectNotifyState : public ULCBaseNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	UNiagaraSystem* NiagaraEffect;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effect")
	FVector Offset = FVector(150.f, 0.f, 0.f);

private:
	UPROPERTY()
	UNiagaraComponent* SpawnedEffect;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
