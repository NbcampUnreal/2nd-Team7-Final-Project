#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "BTService_PatrolRandomPoint.generated.h"

UCLASS()
class LASTCANARY_API UBTService_PatrolRandomPoint : public UBTService
{
	GENERATED_BODY()

public:
    UBTService_PatrolRandomPoint();

    /** 추적 우선 액터 키 (Object) */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetActorKey;

    /** Patrol 목적지 키 (Vector) */
    UPROPERTY(EditAnywhere, Category = "Blackboard")
    FBlackboardKeySelector TargetLocationKey;

    /** 순찰 반경 */
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolRadius = 1000.f;

protected:
    virtual void TickNode(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds
    ) override;
	
};
