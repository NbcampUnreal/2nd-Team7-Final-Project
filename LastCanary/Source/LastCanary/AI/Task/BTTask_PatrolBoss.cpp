#include "AI/Task/BTTask_PatrolBoss.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BTTaskNode.h"
#include "NavigationSystem.h"
#include "GameFramework/Character.h"
#include "NavigationSystem.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_PatrolBoss::UBTTask_PatrolBoss()
{
    NodeName = TEXT("Patrol");
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_PatrolBoss::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    // 1) AIController & Pawn 가져오기
    AAIController* AICon = Cast<AAIController>(OwnerComp.GetAIOwner());
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!Pawn || !AICon)
    {
        return EBTNodeResult::Failed;
    }

    // 2) 랜덤 Patrol 지점 계산 (Min~Max 반경, 최소거리 보장)
    FVector Origin = Pawn->GetActorLocation();
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
    {
        return EBTNodeResult::Failed;
    }

    // 3) Blackboard 에 PatrolLocation 저장
    OwnerComp.GetBlackboardComponent()
        ->SetValueAsVector(TEXT("PatrolLocation"), RandomPoint.Location);

    // 4) FAIMoveRequest 생성 및 설정
    FAIMoveRequest MoveReq(RandomPoint.Location);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    MoveReq.SetAllowPartialPath(true);  // 부분 경로도 허용

    // 5) MoveTo 요청 및 RequestID 획득
    FNavPathSharedPtr OutPath;
    FPathFollowingRequestResult MoveResult = AICon->MoveTo(MoveReq, &OutPath);

    // 성공이 아닐 경우 바로 실패 처리
    if (MoveResult.Code != EPathFollowingRequestResult::RequestSuccessful
        && MoveResult.Code != EPathFollowingRequestResult::AlreadyAtGoal)
    {
        return EBTNodeResult::Failed;
    }

    // RequestID는 MoveResult.MoveId에 들어 있습니다
    FAIRequestID RequestID = MoveResult.MoveId;

    // 6) 작업 완료 콜백 바인딩
    UBehaviorTreeComponent* BTCompPtr = &OwnerComp;
    UPathFollowingComponent* PathComp = AICon->GetPathFollowingComponent();
    if (PathComp)
    {
        PathComp->OnRequestFinished.AddLambda(
            [this, BTCompPtr, RequestID, PathComp]
            (FAIRequestID ID, const FPathFollowingResult& Result)
            {
                if (ID != RequestID) return;

                PathComp->OnRequestFinished.RemoveAll(this);

                EBTNodeResult::Type FinishResult =
                    Result.IsSuccess()
                    ? EBTNodeResult::Succeeded
                    : EBTNodeResult::Failed;

                FinishLatentTask(*BTCompPtr, FinishResult);
            });
    }
    else
    {
        // PathComp가 없으면 즉시 실패 처리
        return EBTNodeResult::Failed;
    }

    // 7) 이동 완료까지 InProgress 반환
    return EBTNodeResult::InProgress;
}