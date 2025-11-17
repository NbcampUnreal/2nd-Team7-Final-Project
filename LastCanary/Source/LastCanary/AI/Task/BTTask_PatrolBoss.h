#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "AIController.h"
#include "NavigationSystem.h"
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
        UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
    /** 순찰 반경: 이 값 안에서 랜덤 거리 뽑음 */
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MinPatrolRadius = 1000.f;

    UPROPERTY(EditAnywhere, Category = "Patrol")
    float MaxPatrolRadius = 3000.f;

    /** 도달로 간주할 거리 */
    UPROPERTY(EditAnywhere, Category = "Patrol")
    float AcceptanceRadius = 150.f;

private:
    // 이동 요청 ID 보관
    FAIRequestID CurrentRequestID;
};