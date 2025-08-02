#pragma once

#include "CoreMinimal.h"
#include "Character/AnimNotify/LCBaseNotify.h"
#include "LCSoundNotify.generated.h"

UCLASS()
class LASTCANARY_API ULCSoundNotify : public ULCBaseNotify
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundToPlay;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

};
