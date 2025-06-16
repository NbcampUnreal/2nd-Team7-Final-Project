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
    LastSoundTime = 0.0f;
    CurrentOwnerComp = nullptr;
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
        return EBTNodeResult::Failed;
    }

    ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn());
    if (Monster)
    {
        MyAcceptableRadius = Monster->GetAttackRange();

        Monster->MulticastAIMove();
        Monster->PlayChaseSound();
        LastSoundTime = GetWorld()->GetTimeSeconds();
    }

    CurrentOwnerComp = &OwnerComp;

    GetWorld()->GetTimerManager().SetTimer(
        MoveCheckTimer,
        this,
        &UBTTask_Move::CheckMoveStatus,
        0.1f,
        true//반복
    );

    return EBTNodeResult::InProgress;
}

void UBTTask_Move::CheckMoveStatus()
{
    if (!CurrentOwnerComp)
    {
        // 타이머 정리
        if (GetWorld())
        {
            GetWorld()->GetTimerManager().ClearTimer(MoveCheckTimer);
        }
        return;
    }

    AAIController* AIController = CurrentOwnerComp->GetAIOwner();
    UBlackboardComponent* BlackboardComp = CurrentOwnerComp->GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        // 타이머 정리 후 실패
        GetWorld()->GetTimerManager().ClearTimer(MoveCheckTimer);
        FinishLatentTask(*CurrentOwnerComp, EBTNodeResult::Failed);
        CurrentOwnerComp = nullptr;
        return;
    }

    // 주기적으로 사운드 재생
    float CurrentTime = GetWorld()->GetTimeSeconds();
    if (CurrentTime - LastSoundTime >= SoundTimer)
    {
        if (ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn()))
        {
            Monster->PlayChaseSound();
            LastSoundTime = CurrentTime;
        }
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
    if (!TargetActor)
    {
        ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
        if (BaseAIController)
        {
            AIController->StopMovement();
            BaseAIController->SetPatrolling();
        }

        // 타이머 정리 후 실패
        GetWorld()->GetTimerManager().ClearTimer(MoveCheckTimer);
        FinishLatentTask(*CurrentOwnerComp, EBTNodeResult::Failed);
        CurrentOwnerComp = nullptr;
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
        if (DistanceToTarget <= MyAcceptableRadius + 50)
        {
            FVector DirectionToTarget = (TargetActor->GetActorLocation() - ControlledPawn->GetActorLocation()).GetSafeNormal();
            FRotator TargetRotation = DirectionToTarget.Rotation();
            ControlledPawn->SetActorRotation(TargetRotation);

            ABaseAIController* BaseAIController = Cast<ABaseAIController>(CurrentOwnerComp->GetAIOwner());
            if (BaseAIController)
            {
                BaseAIController->SetAttacking();
            }

            GetWorld()->GetTimerManager().ClearTimer(MoveCheckTimer);
            FinishLatentTask(*CurrentOwnerComp, EBTNodeResult::Succeeded);
            CurrentOwnerComp = nullptr;
        }
    }
}