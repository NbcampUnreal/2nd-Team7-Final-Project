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

    virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;

protected:
    void CheckPatrolStatus(UBehaviorTreeComponent* OwnerComp);

    void CleanupPatrolTimer(UBehaviorTreeComponent* OwnerComp);

public:
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MinDistance = 300.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MaxDistance = 800.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float AcceptableRadius = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float Delay = 2.0f;

private:
    TMap<TWeakObjectPtr<UBehaviorTreeComponent>, FTimerHandle> PatrolTimerMap;

    TMap<TWeakObjectPtr<UBehaviorTreeComponent>, float> EndTimeMap;
};