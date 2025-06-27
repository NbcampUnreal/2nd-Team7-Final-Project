// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "BaseCharacterAttackNotify.generated.h"

UENUM(BlueprintType)
enum class ECharacterAttackType : uint8
{
    None        UMETA(DisplayName = "None"),
    Kick    UMETA(DisplayName = "Kick"),
    Punch     UMETA(DisplayName = "Punch")
};


UENUM(BlueprintType)
enum class ECharacterAttackTimingType : uint8
{
    None        UMETA(DisplayName = "None"),
    Trigger    UMETA(DisplayName = "Attack Start"),
    End     UMETA(DisplayName = "Attack End")
};


UCLASS()
class LASTCANARY_API UBaseCharacterAttackNotify : public UAnimNotify
{
	GENERATED_BODY()
	
public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
    ECharacterAttackType NotifyType = ECharacterAttackType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
    ECharacterAttackTimingType AttackTimingType = ECharacterAttackTimingType::None;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sound")
    USoundBase* SoundToPlay;
public:
    // Editor에서 설정할 수 있도록 TargetActor 지정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Notify")
    AActor* TargetActor;

    virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;

    void StartAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
    void EndAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation);
};
