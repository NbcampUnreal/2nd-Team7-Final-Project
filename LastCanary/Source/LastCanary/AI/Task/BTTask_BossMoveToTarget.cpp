#include "AI/Task/BTTask_BossMoveToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"

UBTTask_BossMoveToTarget::UBTTask_BossMoveToTarget(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Boss Move To Target");
    bNotifyTick = true;          // ExecuteTask 후 TickTask 를 호출
    bCreateNodeInstance = true;  // 노드별 인스턴스 메모리 유지
}

EBTNodeResult::Type UBTTask_BossMoveToTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
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


    AIController->MoveToActor(TargetActor, MyAcceptableRadius);

    return EBTNodeResult::InProgress;
}

void UBTTask_BossMoveToTarget::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
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
        AAIController* BaseAIController = Cast<AAIController>(AIController);
        if (BaseAIController)
        {
            AIController->StopMovement();

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
        if (DistanceToTarget <= MyAcceptableRadius + 50)
        {
            FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
        }
    }
}