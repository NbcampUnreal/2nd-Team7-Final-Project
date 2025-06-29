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

    TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
    FTimerHandle& TimerHandle = PatrolTimerMap.FindOrAdd(WeakOwnerComp);
    EndTimeMap.FindOrAdd(WeakOwnerComp) = AIController->GetWorld()->GetTimeSeconds() + Delay;

    GetWorld()->GetTimerManager().SetTimer(
        TimerHandle,
        [this, WeakOwnerComp]() {
        if (IsValid(this) && WeakOwnerComp.IsValid())
        {
            this->CheckPatrolStatus(WeakOwnerComp.Get());
        }
        else
        {
            if (IsValid(this))
            {
                PatrolTimerMap.Remove(WeakOwnerComp);
                EndTimeMap.Remove(WeakOwnerComp);
            }
        }
    },
        0.1f, true
    );

    return EBTNodeResult::InProgress;
}

void UBTTask_RandomPatrol::CheckPatrolStatus(UBehaviorTreeComponent* OwnerComp)
{
    if (!OwnerComp) return;

    AAIController* AIController = OwnerComp->GetAIOwner();
    if (!AIController)
    {
        CleanupPatrolTimer(OwnerComp);
        FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
        return;
    }

    UBlackboardComponent* BlackboardComp = OwnerComp->GetBlackboardComponent();
    if (!BlackboardComp)
    {
        CleanupPatrolTimer(OwnerComp);
        FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
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

            CleanupPatrolTimer(OwnerComp);
            FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
            return;
        }
    }

    float* EndTime = EndTimeMap.Find(OwnerComp);
    if (EndTime && AIController->GetWorld()->GetTimeSeconds() >= *EndTime)
    {
        AIController->StopMovement();
        CleanupPatrolTimer(OwnerComp);
        FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_RandomPatrol::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
    CleanupPatrolTimer(&OwnerComp);

    Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_RandomPatrol::CleanupPatrolTimer(UBehaviorTreeComponent* OwnerComp)
{
    if (!OwnerComp) return;

    TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = OwnerComp;
    FTimerHandle* TimerHandle = PatrolTimerMap.Find(WeakOwnerComp);
    if (TimerHandle && TimerHandle->IsValid())
    {
        GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
    }

    PatrolTimerMap.Remove(WeakOwnerComp);
    EndTimeMap.Remove(WeakOwnerComp);
}