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
    /** 보스가 목표보다 얼마나 멀어지면 텔레포트할지 */
    UPROPERTY(EditAnywhere, Category = "Teleport")
    float TeleportDistance = 2000.f;

    /** 목표와 높이 차이가 얼마나 나면 점프 대신 점프+앞으로 이동할지 */
    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpHeightThreshold = 100.f;

    /** 점프 시 앞으로 밀어줄 속도 (XY 평면 크기) */
    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpForwardStrength = 600.f;

    /** 점프 시 위로 띄우는 속도 (Z축 크기) */
    UPROPERTY(EditAnywhere, Category = "Jump")
    float JumpUpStrength = 800.f;

    /** AcceptableRadius */
    UPROPERTY(EditAnywhere, Category = "Movement")
    float MyAcceptableRadius = 100.f;
	
};  
