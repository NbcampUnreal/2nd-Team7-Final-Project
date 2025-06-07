// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BaseCharacterAnimNotify.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UBaseCharacterAnimNotify : public UAnimNotify
{
	GENERATED_BODY()
public:
    // Editor에서 설정할 수 있도록 TargetActor 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
    AActor* TargetActor;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
