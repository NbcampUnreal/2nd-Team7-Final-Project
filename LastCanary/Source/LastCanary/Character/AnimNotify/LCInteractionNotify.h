// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AnimNotify/LCBaseNotify.h"
#include "LCInteractionNotify.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ULCInteractionNotify : public ULCBaseNotify
{
	GENERATED_BODY()
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
