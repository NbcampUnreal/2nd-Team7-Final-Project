#include "AI/Task/BTTask_PatrolBoss.h"
#include "AIController.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"

UBTTask_PatrolBoss::UBTTask_PatrolBoss(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Patrol Boss");
    PatrolRadius = 1000.f;
    WaitTime = 3.f;
    bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_PatrolBoss::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    FPatrolBossMemory* Memory = reinterpret_cast<FPatrolBossMemory*>(NodeMemory);
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!AICon || !Pawn) return EBTNodeResult::Failed;

    // 순찰 시작 위치 저장
    Memory->Origin = Pawn->GetActorLocation();
    Memory->bWaiting = false;
    Memory->WaitRemaining = WaitTime;

    // 랜덤 지점 생성
    FNavLocation NavLoc;
    UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
    if (!NavSys || !NavSys->GetRandomPointInNavigableRadius(Memory->Origin, PatrolRadius, NavLoc))
        return EBTNodeResult::Failed;

    Memory->Destination = NavLoc.Location;

    // 이동 시작
    AICon->MoveToLocation(Memory->Destination, 50.f, true, true, false, false, 0, true);
    return EBTNodeResult::InProgress;
}

void UBTTask_PatrolBoss::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    FPatrolBossMemory* Memory = reinterpret_cast<FPatrolBossMemory*>(NodeMemory);
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!AICon || !Pawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    if (!Memory->bWaiting)
    {
        // 이동 중 도착 판정
        float Dist = FVector::Dist(Pawn->GetActorLocation(), Memory->Destination);
        if (Dist <= 100.f)
        {
            Memory->bWaiting = true;
            Memory->WaitRemaining = WaitTime;
            AICon->StopMovement();
        }
    }
    else
    {
        // 대기 상태
        Memory->WaitRemaining -= DeltaSeconds;
        if (Memory->WaitRemaining <= 0.f)
        {
            // 다음 목적지 설정
            FNavLocation NavLoc;
            UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
            if (NavSys && NavSys->GetRandomPointInNavigableRadius(Memory->Origin, PatrolRadius, NavLoc))
            {
                Memory->Destination = NavLoc.Location;
                AICon->MoveToLocation(Memory->Destination, 50.f, true, true, false, false, 0, true);
                Memory->bWaiting = false;
                Memory->WaitRemaining = WaitTime;
            }
        }
    }
}

EBTNodeResult::Type UBTTask_PatrolBoss::AbortTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        AICon->StopMovement();
    }
    return EBTNodeResult::Aborted;
}
