#pragma once

#include "CoreMinimal.h"
#include "Character/AnimNotify/LCBaseNotify.h"
#include "NiagaraSystem.h"
#include "LCEffectNotify.generated.h"

UCLASS()
class LASTCANARY_API ULCEffectNotify : public ULCBaseNotify
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	UNiagaraSystem* NiagaraEffect;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
