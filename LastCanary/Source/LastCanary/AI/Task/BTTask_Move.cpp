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
        Monster->MulticastAIMove();

    AIController->MoveToActor(TargetActor, MyAcceptableRadius);

    return EBTNodeResult::InProgress;
}

void UBTTask_Move::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    AActor* TargetActor = Cast<AActor>(BlackboardComp->GetValueAsObject("TargetActor"));
    if (!TargetActor)
    {
        ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController);
        if (BaseAIController)
        {
            AIController->StopMovement();

            BaseAIController->SetPatrolling();

            //UE_LOG(LogTemp, Warning, TEXT("Miss Target"));

            FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
            return;
        }
    }

    //이동중인가?
    EPathFollowingStatus::Type MoveStatus = AIController->GetMoveStatus();

    if (MoveStatus == EPathFollowingStatus::Idle)
    {
        //다시 추적
        AIController->MoveToActor(TargetActor, MyAcceptableRadius);
    }

    APawn* ControlledPawn = AIController->GetPawn();
    if (ControlledPawn)
    {
        float DistanceToTarget = FVector::Distance(ControlledPawn->GetActorLocation(), TargetActor->GetActorLocation());
        if (DistanceToTarget <= MyAcceptableRadius+50)
        {
            ABaseAIController* BaseAIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
            if (BaseAIController)
            {
                BaseAIController->SetAttacking();
            }
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}

//BaseMonsterCharacter->ChaseSound sound 플레이 하기