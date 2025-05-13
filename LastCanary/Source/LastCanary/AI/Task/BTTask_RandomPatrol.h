#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_RandomPatrol.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_RandomPatrol : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_RandomPatrol();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolRadius = 1000.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    bool bPatrolAroundSpawnPoint = true;

    UPROPERTY(EditAnywhere, Category = "Patrol", meta = (EditCondition = "!bPatrolAroundSpawnPoint"))
    bool bPatrolAroundCurrentLocation = false;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    FBlackboardKeySelector PatrolLocationKey;
};