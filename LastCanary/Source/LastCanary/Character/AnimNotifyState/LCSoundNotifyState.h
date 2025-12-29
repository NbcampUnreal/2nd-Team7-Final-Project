// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AnimNotifyState/LCBaseNotifyState.h"
#include "LCSoundNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ULCSoundNotifyState : public ULCBaseNotifyState
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	USoundBase* SoundToPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
	bool bLoopSound = false;

private:
	UPROPERTY()
	UAudioComponent* PlayingAudioComponent;

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
