#include "AI/Task/BTTask_Move.h"
#include "AI/BaseAIController.h"
#include "AI/BaseMonsterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_Move::UBTTask_Move()
{
	NodeName = TEXT("Move to Target");
	bNotifyTick = true;
	SoundTimer = 2.5f;
}

EBTNodeResult::Type UBTTask_Move::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();
	if (!BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
	if (!TargetActor)
	{
		ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
		if (BaseAIController)
		{
			int32 CurrentState = BlackboardComp->GetValueAsInt("State");
			if (CurrentState != 4)
			{
				AIController->StopMovement();
				BaseAIController->SetPatrolling();
			}
		}
		return EBTNodeResult::Failed;
	}

	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
	FTimerHandle& TimerHandle = MoveTimerMap.FindOrAdd(WeakOwnerComp);
	LastSoundTimeMap.FindOrAdd(WeakOwnerComp) = GetWorld()->GetTimeSeconds();

	ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn());
	if (Monster)
	{
		MyAcceptableRadius = Monster->GetAttackRange();
		Monster->MulticastAIMove();
		Monster->PlayChaseSound();
	}

	GetWorld()->GetTimerManager().SetTimer(
		TimerHandle,
		[this, WeakOwnerComp]() {
		if (IsValid(this) && WeakOwnerComp.IsValid())
		{
			this->CheckMoveStatus(WeakOwnerComp.Get());
		}
		else
		{
			if (IsValid(this))
			{
				MoveTimerMap.Remove(WeakOwnerComp);
				LastSoundTimeMap.Remove(WeakOwnerComp);
			}
		}
	},
		0.1f,
		true
	);

	return EBTNodeResult::InProgress;
}

void UBTTask_Move::CheckMoveStatus(UBehaviorTreeComponent* OwnerComp)
{
	if (!OwnerComp) return;

	AAIController* AIController = OwnerComp->GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp->GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		CleanupTimer(OwnerComp);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 주기적으로 사운드 재생
	float CurrentTime = GetWorld()->GetTimeSeconds();
	float* LastSoundTime = LastSoundTimeMap.Find(OwnerComp);
	if (LastSoundTime && CurrentTime - *LastSoundTime >= SoundTimer)
	{
		if (ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn()))
		{
			Monster->PlayChaseSound();
			*LastSoundTime = CurrentTime;
		}
	}

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
	if (!TargetActor)
	{
		ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
		if (BaseAIController)
		{
			int32 CurrentState = BlackboardComp->GetValueAsInt("State");
			if (CurrentState != 4)
			{
				AIController->StopMovement();
				BaseAIController->SetPatrolling();
			}
		}

		CleanupTimer(OwnerComp);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 이동중인가?
	EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Idle)
	{
		// 다시 추적
		AIController->MoveToActor(TargetActor, MyAcceptableRadius);
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (ControlledPawn)
	{
		float DistanceToTarget = FVector::Distance(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
		if (DistanceToTarget <= MyAcceptableRadius + 70)
		{
			FVector DirectionToTarget = (TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
			FRotator TargetRotation = DirectionToTarget.Rotation();
			ControlledPawn->SetActorRotation(TargetRotation);

			ABaseAIController* BaseAIController = Cast<ABaseAIController>(OwnerComp->GetAIOwner());
			if (BaseAIController)
			{
				BaseAIController->SetAttacking();
			}

			CleanupTimer(OwnerComp);
			FinishLatentTask(*OwnerComp, EBTNodeResult::Succeeded);
		}
	}
}

void UBTTask_Move::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	CleanupTimer(&OwnerComp);

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

void UBTTask_Move::CleanupTimer(UBehaviorTreeComponent* OwnerComp)
{
	if (!OwnerComp) return;

	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = OwnerComp;
	FTimerHandle* TimerHandle = MoveTimerMap.Find(WeakOwnerComp);
	if (TimerHandle && TimerHandle->IsValid())
	{
		GetWorld()->GetTimerManager().ClearTimer(*TimerHandle);
	}
	MoveTimerMap.Remove(WeakOwnerComp);
	LastSoundTimeMap.Remove(WeakOwnerComp);
}