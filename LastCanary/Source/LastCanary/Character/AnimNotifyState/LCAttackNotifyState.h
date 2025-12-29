// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/AnimNotifyState/LCBaseNotifyState.h"
#include "Character/Component/CharacterAttackComponent.h"
#include "LCAttackNotifyState.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ULCAttackNotifyState : public ULCBaseNotifyState
{
	GENERATED_BODY()
public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
	EAttackType AttackType = EAttackType::None;

private:
	void HandleStartAttackByType(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration);
	void HandleEndAttackByType(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
};
