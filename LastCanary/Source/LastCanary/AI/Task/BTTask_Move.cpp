#include "AI/Task/BTTask_Move.h"
#include "AI/BaseAIController.h"
#include "AI/BaseMonsterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"
#include "Navigation/PathFollowingComponent.h"
#include "NavigationSystem.h" // UNavigationSystemV1을 위해 추가

UBTTask_Move::UBTTask_Move()
{
	NodeName = TEXT("Move to Target");
}

EBTNodeResult::Type UBTTask_Move::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	if (!AIController)
	{
		return EBTNodeResult::Failed;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return EBTNodeResult::Failed;
	}

	APawn* AIPawn = AIController->GetPawn();
	if (!AIPawn)
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

	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
	if (!TargetActor)
	{
		ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
		if (BaseAIController)
		{
			AIController->StopMovement();
		}
		return EBTNodeResult::Failed;
	}

	ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIPawn);
	if (Monster)
	{
		MyAcceptableRadius = Monster->GetAttackRange();
		Monster->MulticastAIMove();
		Monster->PlayChaseSound();
	}

	AIController->MoveToActor(TargetActor, MyAcceptableRadius);

	TWeakObjectPtr<UBTTask_Move> WeakThis = this;
	TWeakObjectPtr<UBehaviorTreeComponent> WeakOwnerComp = &OwnerComp;
	FTimerHandle& TimerHandle = MoveTimerMap.FindOrAdd(WeakOwnerComp);

	LastSoundTimeMap.FindOrAdd(WeakOwnerComp) = World->GetTimeSeconds();

	World->GetTimerManager().SetTimer(
		TimerHandle,
		[WeakThis, WeakOwnerComp]() {
		if (WeakThis.IsValid())
		{
			if (WeakOwnerComp.IsValid())
			{
				WeakThis->CheckMoveStatus(WeakOwnerComp.Get());
			}
			else
			{
				WeakThis->MoveTimerMap.Remove(WeakOwnerComp);
				WeakThis->LastSoundTimeMap.Remove(WeakOwnerComp);
			}
		}
	},
		0.1f, // 0.1초마다 체크
		true
	);

	return EBTNodeResult::InProgress; // 비동기 작업이므로 InProgress 반환
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

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	// 주기적으로 사운드 재생
	float CurrentTime = World->GetTimeSeconds();
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
		// 타겟이 없어지면 태스크 실패
		AIController->StopMovement(); // 혹시 이동 중이었다면 멈춤
		CleanupTimer(OwnerComp);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	APawn* ControlledPawn = AIController->GetPawn();
	if (!ControlledPawn)
	{
		CleanupTimer(OwnerComp);
		FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 목표와 AI 사이의 거리를 계산합니다.
	float DistanceToTarget = FVector::Distance(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());

	if (DistanceToTarget <= MyAcceptableRadius + 70.0f) // 70f는 여유 거리
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
		return;
	}

	EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();
	if (MoveStatus == EPathFollowingStatus::Idle)
	{
		EPathFollowingRequestResult::Type MoveResult = AIController->MoveToActor(TargetActor, MyAcceptableRadius);

		if (MoveResult == EPathFollowingRequestResult::Failed)
		{
			FNavLocation ProjectedNavLocation;

			if (UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World))
			{
				if (NavSystem->ProjectPointToNavigation(
					TargetActor->GetActorLocation(),       
					ProjectedNavLocation,                   
					FVector(1000.f, 1000.f, 1000.f),       
					&AIController->GetNavAgentPropertiesRef(), 
					nullptr
				))
				{
					AIController->MoveToLocation(ProjectedNavLocation.Location, MyAcceptableRadius);
				}
				else
				{
					AIController->StopMovement();
					CleanupTimer(OwnerComp);
					FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
					return;
				}
			}
			else // NavSystem 자체를 찾지 못한 경우
			{
				AIController->StopMovement();
				CleanupTimer(OwnerComp);
				FinishLatentTask(*OwnerComp, EBTNodeResult::Failed);
				return;
			}
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