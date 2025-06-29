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
}

EBTNodeResult::Type UBTTask_RandomPatrol::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn)
	{
		return EBTNodeResult::Failed;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	if (AIPawn->GetLocalRole() != ROLE_Authority)
	{
		return EBTNodeResult::Succeeded;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	FVector CurrentLocation = AIPawn->GetActorLocation();
	float RandomAngle = FMath::RandRange(0.0f, 2.0f * PI);
	FVector Direction(FMath::Cos(RandomAngle), FMath::Sin(RandomAngle), 0.0f);
	Direction.Normalize();

	float Distance = FMath::RandRange(MinDistance, MaxDistance);
	FVector TargetLocation = CurrentLocation + Direction * Distance;

	ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIPawn);
	if (Monster)
	{
		Monster->MulticastAIMove();
		AIController->MoveToLocation(TargetLocation, AcceptableRadius);
	}
	else
	{
		AIController->MoveToLocation(TargetLocation, AcceptableRadius);
	}

	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
	FTimerHandle& TimerHandle = PatrolTimerMap.FindOrAdd(WeakOwnerComp);
	EndTimeMap.FindOrAdd(WeakOwnerComp) = World->GetTimeSeconds() + Delay;

	//This를 직접 남게 하지 말고 대신 남아있게 할 것
	TWeakObjectPtr<UBTTask_RandomPatrol> WeakThis = this;

	World->GetTimerManager().SetTimer(
		TimerHandle,
		[WeakThis, WeakOwnerComp]()
	{
		//Task(this)있고
		if (WeakThis.IsValid())
		{
			//OwnerComp있으면
			if (WeakOwnerComp.IsValid())
			{
				WeakThis->CheckPatrolStatus(WeakOwnerComp.Get());
			}
			else
			{
				//OwnerComp없으면 정리
				WeakThis->PatrolTimerMap.Remove(WeakOwnerComp);
				WeakThis->EndTimeMap.Remove(WeakOwnerComp);
			}
		}
	},
		0.1f,
		true
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

	UWorld* World = GetWorld();
	if (EndTime && World && World->GetTimeSeconds() >= *EndTime)
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

	UWorld* World = GetWorld();
	if (World && TimerHandle && TimerHandle->IsValid())
	{
		World->GetTimerManager().ClearTimer(*TimerHandle);
	}

	PatrolTimerMap.Remove(WeakOwnerComp);
	EndTimeMap.Remove(WeakOwnerComp);
}