#include "AI/Task/BTTask_RandomPatrol.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "AI/BaseMonsterCharacter.h"

UBTTask_RandomPatrol::UBTTask_RandomPatrol()
{
	NodeName = TEXT("Random Patrol");

	bNotifyTick = true;
}

EBTNodeResult::Type UBTTask_RandomPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController) return EBTNodeResult::Failed;

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn) return EBTNodeResult::Failed;

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp) return EBTNodeResult::Failed;

	FVector CurrentLocation = AIPawn->GetActorLocation();

	float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
	FVector Direction(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);
	Direction.Normalize();

	float Distance = FMath::RandRange(MinDistance, MaxDistance);

	FVector TargetLocation = CurrentLocation + Direction * Distance;

	ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn());
	if (Monster)
	{
		Monster->MulticastAIMove();
		AIController->MoveToLocation(TargetLocation, AcceptableRadius);
	}

	EndTime = AIController->GetWorld()->GetTimeSeconds() + Delay;

	return EBTNodeResult::InProgress;
}

void UBTTask_RandomPatrol::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    if (!AIController)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
    if (!BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
    if (TargetActor)
    {
        ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
        if (BaseAIController)
        {
            AIController->StopMovement();

            BaseAIController->SetChasing(TargetActor);

            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    if (AIController->GetWorld()->GetTimeSeconds() >= EndTime)
    {
        AIController->StopMovement();

        /*ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn());
        if (Monster)
        {
            Monster->MulticastAIDeath();
        }*/

        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}