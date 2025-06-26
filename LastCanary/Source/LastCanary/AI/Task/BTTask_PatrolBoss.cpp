#include "AI/Task/BTTask_PatrolBoss.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"                      // UNavigationSystemV1
#include "Navigation/PathFollowingComponent.h"     // UPathFollowingComponent, FPathFollowingResult

UBTTask_PatrolBoss::UBTTask_PatrolBoss()
{
    NodeName = TEXT("Patrol");
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_PatrolBoss::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    // 1) AIController & Pawn
    AAIController* AICon = Cast<AAIController>(OwnerComp.GetAIOwner());
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!Pawn || !AICon)
        return EBTNodeResult::Failed;

    // 2) 랜덤 지점 계산 (Min, Max Patrol Radius 사이, 최소거리 보장)
    const FVector Origin = Pawn->GetActorLocation();
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
    FNavLocation RandomPoint;
    bool bFound = false;
    for (int i = 0; i < 10; ++i)
    {
        float R = FMath::RandRange(MinPatrolRadius, MaxPatrolRadius);
        if (NavSys->GetRandomReachablePointInRadius(Origin, R, RandomPoint) &&
            FVector::Dist(Origin, RandomPoint.Location) >= MinPatrolRadius)
        {
            bFound = true;
            break;
        }
    }
    if (!bFound)
        return EBTNodeResult::Failed;

    // 3) Blackboard 에 위치 저장
    OwnerComp.GetBlackboardComponent()
        ->SetValueAsVector(TEXT("PatrolLocation"), RandomPoint.Location);

    // 4) MoveTo 요청 & 실제 RequestID 얻기
    AICon->MoveToLocation(RandomPoint.Location, AcceptanceRadius);
    FAIRequestID RequestID = AICon->GetCurrentMoveRequestID();

    // 5) 안전하게 포인터 복사
    UBehaviorTreeComponent* BTCompPtr = &OwnerComp;
    auto* PathComp = AICon->GetPathFollowingComponent();

    // 6) 이동 완료 콜백 바인딩
    PathComp->OnRequestFinished.AddLambda(
        [this, BTCompPtr, RequestID, PathComp]
        (FAIRequestID ID, const FPathFollowingResult& Result)
        {
            // 같은 요청인지 확인
            if (ID != RequestID)
                return;

            // 언바인딩
            PathComp->OnRequestFinished.RemoveAll(this);

            // 성공/실패 판정
            const EBTNodeResult::Type FinishResult =
                Result.IsSuccess() ? EBTNodeResult::Succeeded
                : EBTNodeResult::Failed;

            // Task 종료 알림
            FinishLatentTask(*BTCompPtr, FinishResult);
        });

    // 7) 이동 완료 때까지 기다립니다
    return EBTNodeResult::InProgress;
}