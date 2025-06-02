#include "AI/Task/BTTask_MoveToChaseLocation.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_MoveToChaseLocation::UBTTask_MoveToChaseLocation(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Move To Chase Location");
    AcceptanceRadius = 150.f;
}

EBTNodeResult::Type UBTTask_MoveToChaseLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;

    const FVector Dest = OwnerComp
        .GetBlackboardComponent()
        ->GetValueAsVector(TEXT("ChaseLocation"));

    AICon->MoveToLocation(
        Dest,
        AcceptanceRadius,
        true,
        true,
        false,
        false,
        0,
        true
    );

    return EBTNodeResult::Succeeded;
}
