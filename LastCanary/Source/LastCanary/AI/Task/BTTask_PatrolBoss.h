#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PatrolBoss.generated.h"


struct FPatrolBossMemory
{
    FVector Destination;
};

UCLASS()
class LASTCANARY_API UBTTask_PatrolBoss : public UBTTaskNode
{
    GENERATED_BODY()

public:
    UBTTask_PatrolBoss();

    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

protected:
    // 순찰 반경
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float PatrolRadius;

    // 수용 반경
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float AcceptanceRadius;
};