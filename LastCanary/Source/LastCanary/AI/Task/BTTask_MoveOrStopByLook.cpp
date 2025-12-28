// Fill out your copyright notice in the Description page of Project Settings.


#include "AI/Task/BTTask_MoveOrStopByLook.h"

#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/Actor.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_MoveOrStopByLook::UBTTask_MoveOrStopByLook()
{
	NodeName = TEXT("Move Or Stop By Look (Tick)");

	bNotifyTick = true;  // TickTask ȣ���� ���� �ʿ�

	IsAnyPlayerLookingKey.AddBoolFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveOrStopByLook, IsAnyPlayerLookingKey));
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(UBTTask_MoveOrStopByLook, TargetActorKey), AActor::StaticClass());
}

uint16 UBTTask_MoveOrStopByLook::GetInstanceMemorySize() const
{
	return sizeof(FTaskMemory);
}

EBTNodeResult::Type UBTTask_MoveOrStopByLook::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	FTaskMemory* Memory = (FTaskMemory*)NodeMemory;
	Memory->bWasStopped = false;
	Memory->TimeSinceLastMove = MinMoveRequestInterval; // �������ڸ��� MoveTo �����ϵ���

	return EBTNodeResult::InProgress;
}

void UBTTask_MoveOrStopByLook::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FTaskMemory* Memory = (FTaskMemory*)NodeMemory;
	Memory->TimeSinceLastMove += DeltaSeconds;

	AAIController* AIController = OwnerComp.GetAIOwner();
	UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

	if (!AIController || !BlackboardComp)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	const bool bIsAnyPlayerLooking = BlackboardComp->GetValueAsBool(IsAnyPlayerLookingKey.SelectedKeyName);

	// 1) ������ �ٶ󺸸� -> ����
	if (bIsAnyPlayerLooking)
	{
		if (!Memory->bWasStopped)
		{
			AIController->StopMovement();
			Memory->bWasStopped = true;
		}
		return;
	}

	// 2) 아무도 안 바라보면 -> 타겟으로 이동
	AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject(TargetActorKey.SelectedKeyName));
	if (!TargetActor)
	{
		// 타겟이 없으면 대기 (Failed로 끝내지 않고 InProgress 유지)
		// 다음 틱에서 타겟이 설정될 때까지 기다림
		return;
	}

	// �̹� ����� ������ �������� ����(���ϸ� ��� �����ϵ��� InProgress ������ ����)
	const APawn* ControlledPawn = AIController->GetPawn();
	if (ControlledPawn)
	{
		const float DistSquared = FVector::DistSquared(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
		if (DistSquared <= FMath::Square(AcceptanceRadius))
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
			return;
		}
	}

	// Stop ���¿��ٰ� Ǯ�� ����, �Ǵ� ���� �ð����� MoveTo ���û
	const bool bShouldRequestMove =
		Memory->bWasStopped ||
		(Memory->TimeSinceLastMove >= MinMoveRequestInterval);

	if (bShouldRequestMove)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(TargetActor);
		MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
		MoveRequest.SetUsePathfinding(true);
		MoveRequest.SetAllowPartialPath(true);
		MoveRequest.SetCanStrafe(true);

		FNavPathSharedPtr OutPath;
		const FPathFollowingRequestResult Result = AIController->MoveTo(MoveRequest, &OutPath);

		// ��û ����/���п� ���� ó��
		if (Result.Code == EPathFollowingRequestResult::Failed)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
			return;
		}

		Memory->bWasStopped = false;
		Memory->TimeSinceLastMove = 0.0f;
	}
}