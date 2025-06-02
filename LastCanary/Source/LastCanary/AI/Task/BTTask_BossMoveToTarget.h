#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/Blackboard/BlackboardKeyType_Object.h"
#include "Navigation/PathFollowingComponent.h"
#include "BTTask_BossMoveToTarget.generated.h"

// 태스크 인스턴스 메모리 구조체: 이동 시점과 목표 액터를 저장
struct FBossMoveToTargetMemory
{
    /** 목표 액터 */
    TWeakObjectPtr<AActor> TargetActor;

    /** MoveTo 호출 여부 체크 */
    bool bMoveRequested = false;
};

UCLASS()
class LASTCANARY_API UBTTask_BossMoveToTarget : public UBTTaskNode
{
	GENERATED_BODY()

public:
    UBTTask_BossMoveToTarget(const FObjectInitializer& ObjInit);

    virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
    virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MyAcceptableRadius = 50.0f;
	
};  
