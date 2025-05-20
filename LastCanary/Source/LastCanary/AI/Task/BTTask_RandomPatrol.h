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

    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MinDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float MaxDistance = 600.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float AcceptableRadius = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Movement")
    float Delay = 2.0f;

private:
    FTimerHandle TimerHandle;

    float EndTime = 0.0f;
};