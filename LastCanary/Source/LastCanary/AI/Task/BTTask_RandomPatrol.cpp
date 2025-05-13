#include "AI/Task/BTTask_RandomPatrol.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Character.h"

UBTTask_RandomPatrol::UBTTask_RandomPatrol()
{
    NodeName = TEXT("Random Patrol");
    PatrolRadius = 1000.0f;
    bPatrolAroundSpawnPoint = true;
    bPatrolAroundCurrentLocation = false;
}

EBTNodeResult::Type UBTTask_RandomPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    /*ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    ACharacter* Character = Cast<ACharacter>(AIController->GetPawn());
    if (!Character)
    {
        return EBTNodeResult::Failed;
    }

    UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(GetWorld());
    if (!NavSystem)
    {
        return EBTNodeResult::Failed;
    }

    FVector Origin = Character->GetActorLocation();

    if (bPatrolAroundSpawnPoint)
    {
        Origin = Character->GetActorLocation();
    }
    else if (bPatrolAroundCurrentLocation)
    {
        Origin = Character->GetActorLocation();
    }
    else
    {
        UBlackboardComponent* BlackboardComp = AIController->GetBlackboard();
        if (BlackboardComp && BlackboardComp->IsVectorValueSet(PatrolLocationKey.SelectedKeyName))
        {
            Origin = BlackboardComp->GetValueAsVector(PatrolLocationKey.SelectedKeyName);
        }
    }

    FNavLocation RandomLocation;
    bool bFound = NavSystem->GetRandomReachablePointInRadius(Origin, PatrolRadius, RandomLocation);

    if (!bFound)
    {
        return EBTNodeResult::Failed;
    }

    UBlackboardComponent* BlackboardComp = AIController->GetBlackboard();
    if (BlackboardComp)
    {
        BlackboardComp->SetValueAsVector(ABaseAIController::PatrolLocationKey, RandomLocation.Location);
    }*/

    UE_LOG(LogTemp, Warning, TEXT("Patrol"));

    return EBTNodeResult::Succeeded;
}