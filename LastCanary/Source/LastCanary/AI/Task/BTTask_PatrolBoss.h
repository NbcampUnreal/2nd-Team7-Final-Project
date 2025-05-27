#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_PatrolBoss.generated.h"


struct FPatrolBossMemory
{
    FVector Origin;             // 순찰 시작 위치
    FVector Destination;        // 현재 목표 지점
    float   WaitRemaining;      // 도착 후 대기 시간 남은량
    bool    bWaiting;           // 대기 중인지 여부
};



UCLASS()
class LASTCANARY_API UBTTask_PatrolBoss : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_PatrolBoss(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

    // 순찰 반경
    UPROPERTY(EditAnywhere, Category = "Boss|Patrol")
    float PatrolRadius;

    // 도착 후 대기 시간 (초)
    UPROPERTY(EditAnywhere, Category = "Boss|Patrol")
    float WaitTime;

    // 인스턴스 메모리 크기 지정
    virtual uint16 GetInstanceMemorySize() const override
    {
        return sizeof(FPatrolBossMemory);
    }

protected:
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

    virtual EBTNodeResult::Type AbortTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
	
};
