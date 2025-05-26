#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BTTask_BossMoveToTarget.generated.h"

UCLASS()
class LASTCANARY_API UBTTask_BossMoveToTarget : public UBTTaskNode
{
	GENERATED_BODY()

    UBTTask_BossMoveToTarget(const FObjectInitializer& ObjInit = FObjectInitializer::Get());

    /** 도착으로 간주할 반경 (Default: 150) */
    UPROPERTY(EditAnywhere, Category = "Boss|Movement")
    float AcceptanceRadius;

protected:
    /** Task 시작 시 호출 */
    virtual EBTNodeResult::Type ExecuteTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;

    /** 매 Tick마다 호출 (bNotifyTick=true) */
    virtual void TickTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory,
        float DeltaSeconds) override;

    /** Task가 강제 중단될 때 호출 */
    virtual EBTNodeResult::Type AbortTask(
        UBehaviorTreeComponent& OwnerComp,
        uint8* NodeMemory) override;
	
};
