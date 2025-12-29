// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Bool.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "BTTask_MoveOrStopByLook.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UBTTask_MoveOrStopByLook : public UBTTaskNode
{
	GENERATED_BODY()
public:
	UBTTask_MoveOrStopByLook();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual uint16 GetInstanceMemorySize() const override;

private:
	/** 플레이어가 몬스터를 바라보고 있는지 여부 (bool) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector IsAnyPlayerLookingKey;

	/** 추적할 타겟 액터 (object) */
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	/** MoveTo 완료 판정 거리 */
	UPROPERTY(EditAnywhere, Category = "Move")
	float AcceptanceRadius = 80.0f;

	/** MoveTo를 너무 자주 재요청하지 않도록 최소 재요청 간격 */
	UPROPERTY(EditAnywhere, Category = "Move")
	float MinMoveRequestInterval = 0.25f;

private:
	struct FTaskMemory
	{
		bool bWasStopped = false;        // 직전에 StopMovement 상태였는지
		float TimeSinceLastMove = 0.0f;  // 마지막 MoveTo 요청 이후 누적 시간
	};
};
