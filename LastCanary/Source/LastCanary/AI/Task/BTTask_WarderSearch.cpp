#include "AI/Task/BTTask_WarderSearch.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
//#include "TimerManager.h"

UBTTask_WarderSearch::UBTTask_WarderSearch()
{
	NodeName = TEXT("WarderSearch");
	bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_WarderSearch::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		return EBTNodeResult::Failed;
	}

	FVector BoxVector = BlackboardComp->GetValueAsVector("WarderSearch");
	if (BoxVector == FVector::ZeroVector)
	{
		return EBTNodeResult::Failed;
	}

	AIController->MoveToLocation(BoxVector - 100.f, 100.0f);

	CachedOwnerComp = &OwnerComp;

	if (UWorld* World = AIController->GetWorld())
	{
		World->GetTimerManager().SetTimer(
			CheckTimerHandle,
			this,
			&UBTTask_WarderSearch::CheckArrival,
			0.3f,
			true
		);
	}

	return EBTNodeResult::InProgress;
}

void UBTTask_WarderSearch::CheckArrival()
{
	if (!CachedOwnerComp)
	{
		return;
	}

	AAIController* AIController = CachedOwnerComp->GetAIOwner();
	UBlackboardComponent* BlackboardComp = CachedOwnerComp->GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		StopTimer();
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 이동 중 타겟 발견 시 추격으로 전환
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
	if (TargetActor)
	{
		ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
		if (BaseAIController)
		{
			StopTimer();

			AIController->StopMovement();

			BaseAIController->SetChasing(TargetActor);

			FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	APawn* ControlledPawn = AIController->GetPawn();
	FVector BoxVector = BlackboardComp->GetValueAsVector("WarderSearch");

	if (!ControlledPawn || BoxVector == FVector::ZeroVector)
	{
		StopTimer();
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
		return;
	}

	// 도착 확인
	float Distance = FVector::Distance(ControlledPawn->GetActorLocation(), BoxVector);
	if (Distance <= 300.0f)
	{
		BlackboardComp->ClearValue("WarderSearch");

		if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController))
		{
			BaseAIController->SetPatrolling();
		}

		StopTimer();
		FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
	}
}

void UBTTask_WarderSearch::StopTimer()
{
	if (CachedOwnerComp)
	{
		if (AAIController* AIController = CachedOwnerComp->GetAIOwner())
		{
			if (UWorld* World = AIController->GetWorld())
			{
				World->GetTimerManager().ClearTimer(CheckTimerHandle);
			}
		}
	}
}