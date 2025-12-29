#include "AI/Task/BTTask_TempleElite_Search.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTTask_TempleElite_Search::UBTTask_TempleElite_Search()
{
    NodeName = TEXT("SearchGimmick");

    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_TempleElite_Search::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    AAIController* AIController = OwnerComp.GetAIOwner();
    UBlackboardComponent* BlackboardComp = OwnerComp.GetBlackboardComponent();

    if (!AIController || !BlackboardComp)
    {
        return EBTNodeResult::Failed;
    }

    FVector BoxVector = BlackboardComp->GetValueAsVector("BoxVector");
    if (BoxVector == FVector::ZeroVector)
    {
        return EBTNodeResult::Failed;
    }

    //UE_LOG(LogTemp, Warning, TEXT("BoxVector: %s"), *BoxVector.ToString());

    AIController->MoveToLocation(BoxVector - 100.f, 100.0f);

    CachedOwnerComp = &OwnerComp;
    
    if (UWorld* World = AIController->GetWorld())
    {
        World->GetTimerManager().SetTimer(
            CheckTimerHandle,
            this,
            &UBTTask_TempleElite_Search::CheckArrival,
            0.3f,
            true
        );
    }

    return EBTNodeResult::InProgress;
}

void UBTTask_TempleElite_Search::CheckArrival() 
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
    FVector BoxVector = BlackboardComp->GetValueAsVector("BoxVector");

    if (!ControlledPawn || BoxVector == FVector::ZeroVector)
    {
        StopTimer();
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Failed);
        return;
    }

    float Distance = FVector::Distance(ControlledPawn->GetActorLocation(), BoxVector);
    if (Distance <= 300.0f)
    {
        BlackboardComp->ClearValue("BoxVector");

        if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(AIController))
        {
            BaseAIController->SetPatrolling();
        }

        StopTimer();
        FinishLatentTask(*CachedOwnerComp, EBTNodeResult::Succeeded);
    }
}

void UBTTask_TempleElite_Search::StopTimer()
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