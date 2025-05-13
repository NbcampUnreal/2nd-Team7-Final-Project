#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/Tasks/BTTask_MoveTo.h"
#include "BTTask_Move.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_Move : public UBTTask_MoveTo
{
    GENERATED_BODY()

public:
    UBTTask_Move();

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Move")
    bool bMoveToTarget = true;

    UPROPERTY(EditAnywhere, Category = "Move", meta = (EditCondition = "!bMoveToTarget"))
    FBlackboardKeySelector LocationKey;

    UPROPERTY(EditAnywhere, Category = "Move")
    float MyAcceptableRadius = 50.0f;

    UPROPERTY(EditAnywhere, Category = "Move", meta = (EditCondition = "bMoveToTarget"))
    float TargetApproachRadius = 100.0f;
};
