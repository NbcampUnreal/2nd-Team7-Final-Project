// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "LCWeepingAIController.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCWeepingAIController : public ABaseAIController
{
	GENERATED_BODY()
public:
	ALCWeepingAIController();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AI")
	UAIPerceptionComponent* AIPerceptionComp;

private:
	UPROPERTY()
	UAISenseConfig_Sight* SightConfig;
	FTimerHandle CheckTimerHandle;

	void CheckAllPlayersLooking();

	virtual void OnPossess(APawn* InPawn) override;

	UFUNCTION()
	void OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus);
	
	static constexpr float SightRadius = 1500.0f;
	static constexpr float LoseSightRadius = 2000.0f;
	static constexpr float PeripheralVisionAngleDegrees = 90.0f;
	static constexpr float LookAtDotProductThreshold = 0.85f;
	static constexpr float PlayerCheckInterval = 0.1f;

	/** 플레이어 즉사 범위 (이 거리 이내 + 아무도 안 보고 있으면 즉사) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat", meta = (AllowPrivateAccess = "true"))
	float KillRange = 100.0f;
	
};
