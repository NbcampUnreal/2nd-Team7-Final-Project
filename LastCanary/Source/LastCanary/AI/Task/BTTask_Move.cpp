#include "AI/Task/BTTask_Move.h"
#include "AI/BaseAIController.h"
#include "AI/BaseMonsterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_Move::UBTTask_Move()
{
    NodeName = TEXT("Move to Target");

    MyAcceptableRadius = 50.0f;
    bMoveToTarget = true;
    TargetApproachRadius = 100.0f;
}

EBTNodeResult::Type UBTTask_Move::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
        return EBTNodeResult::Failed;

    FVector MoveLocation;

    if (bMoveToTarget)
    {
        ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn());
        if (Monster)
        {
            Monster->MulticastAIMove();

            float AttackRange = 1.5f;// Monster->GetAttackRange();
            float MoveToDistance = FMath::Max(AttackRange * 0.8f, TargetApproachRadius);

            FVector Direction = (AIController->GetPawn()->GetActorLocation() - MoveLocation).GetSafeNormal();

            MoveLocation += Direction * MoveToDistance;
        }
    }

    // 이동 명령 수행
    EPathFollowingRequestResult::Type Result = AIController->MoveToLocation(
        MoveLocation,
        MyAcceptableRadius,
        true,  // 기존 이동 중지
        true,  // 가능한 부분까지 이동
        false, // 프로젝션 사용 안함
        true   // 경로 탐색 허용
    );

    if (Result == EPathFollowingRequestResult::Failed)
    {
        return EBTNodeResult::Failed;
    }
    else if (Result == EPathFollowingRequestResult::AlreadyAtGoal)
    {

        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::InProgress;
}