#include "AI/Service/BTService_PatrolRandomPoint.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "NavigationSystem.h"
#include "NavAreas/NavArea_Null.h"

UBTService_PatrolRandomPoint::UBTService_PatrolRandomPoint()
{
    NodeName = TEXT("Patrol Random Destination");
    Interval = 1.0f;
    bCreateNodeInstance = true;
}

void UBTService_PatrolRandomPoint::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!BB || !Pawn) return;

    // 1) Combat 우선: TargetActor가 있으면 Patrol은 건너뜀
    if (BB->GetValueAsObject(TargetActorKey.SelectedKeyName))
        return;

    // 2) NavMesh Random Point
    UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(Pawn->GetWorld());
    if (!NavSys) return;

    FNavLocation RandomLoc;
    if (NavSys->GetRandomPointInNavigableRadius(
        Pawn->GetActorLocation(),
        PatrolRadius,
        RandomLoc
    ))
    {
        BB->SetValueAsVector(
            TargetLocationKey.SelectedKeyName,
            RandomLoc.Location
        );
    }
}