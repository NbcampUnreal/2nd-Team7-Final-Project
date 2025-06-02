#include "AI/Task/BTTask_PatrolBoss.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"

UBTTask_PatrolBoss::UBTTask_PatrolBoss()
{
    NodeName = TEXT("Patrol");
    PatrolRadius = 1000.f;
    AcceptanceRadius = 50.f;
}

EBTNodeResult::Type UBTTask_PatrolBoss::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon->GetPawn();
    if (!AICon || !Pawn) return EBTNodeResult::Failed;

    // 랜덤 위치 생성
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
    FNavLocation RandomPoint;
    if (NavSys->GetRandomPointInNavigableRadius(Pawn->GetActorLocation(), PatrolRadius, RandomPoint))
    {
        OwnerComp.GetBlackboardComponent()
            ->SetValueAsVector("PatrolLocation", RandomPoint.Location);
        AICon->MoveToLocation(RandomPoint.Location, AcceptanceRadius);
        return EBTNodeResult::Succeeded;
    }

    return EBTNodeResult::Failed;
}