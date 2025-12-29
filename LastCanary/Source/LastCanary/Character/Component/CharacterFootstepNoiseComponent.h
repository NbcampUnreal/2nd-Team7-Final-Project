// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterFootstepNoiseComponent.generated.h"

UENUM(BlueprintType)
enum class EFootstepState : uint8
{
    None,
    Walk,
    Run,
    Sprint,
    Crouch
};


UCLASS()
class LASTCANARY_API UCharacterFootstepNoiseComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
    UCharacterFootstepNoiseComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    /** 현재 발소리의 강도 (AI 감지 범위 등에서 사용) */
    UFUNCTION(BlueprintCallable, Category = "Noise")
    float GetCurrentFootstepNoiseLevel() const;

    /** 현재 이동 상태에 따른 소음 타입 (내부 추론용) */
    UFUNCTION(BlueprintCallable, Category = "Noise")
    EFootstepState GetCurrentFootstepState() const;

protected:
    float NoiseLevel;
    EFootstepState CurrentFootstepState;

    /** 상태별 발소리 크기 설정 */
    UPROPERTY(EditDefaultsOnly, Category = "Noise")
    float WalkNoise = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Noise")
    float RunNoise = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Noise")
    float CrouchNoise = 10.f;

    /** 캐릭터의 이동 상태를 감지해서 소리 갱신 */
    void UpdateFootstepState();
};
