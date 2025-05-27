#include "AI/Task/BTTask_BossMoveToTarget.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "DrawDebugHelpers.h"

UBTTask_BossMoveToTarget::UBTTask_BossMoveToTarget(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Boss Move To Target (Avoid Obstacles)");
    AcceptanceRadius = 150.f;
    bNotifyTick = true;  // TickTask 사용
}

EBTNodeResult::Type UBTTask_BossMoveToTarget::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;

    FVector TargetLoc = OwnerComp
        .GetBlackboardComponent()
        ->GetValueAsVector(TEXT("TargetLocationKey"));

    EPathFollowingRequestResult::Type MoveResult =
        AICon->MoveToLocation(
            TargetLoc,
            AcceptanceRadius,
            true,   // bStopOnOverlap
            true,   // bUsePathfinding
            false,  // bProjectGoalLocation
            false,  // bAllowPartialPath
            0,      // NavFilterClass
            true    // bAllowStrafe
        );

    return (MoveResult == EPathFollowingRequestResult::RequestSuccessful)
        ? EBTNodeResult::InProgress
        : EBTNodeResult::Failed;
}

void UBTTask_BossMoveToTarget::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickTask(OwnerComp, NodeMemory, DeltaSeconds);

    AAIController* AICon = OwnerComp.GetAIOwner();
    APawn* Pawn = AICon ? AICon->GetPawn() : nullptr;
    if (!AICon || !Pawn)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 장애물 회피 로직
    const float DetectionDistance = 400.f;
    const float AvoidanceRadius = 300.f;
    const float ObstacleSphereRadius = 75.f;

    FVector PawnLoc = Pawn->GetActorLocation();
    FVector ForwardDir = Pawn->GetActorForwardVector();
    FVector TraceStart = PawnLoc;
    FVector TraceEnd = PawnLoc + ForwardDir * DetectionDistance;

    FCollisionQueryParams Params;
    Params.AddIgnoredActor(Pawn);

    FHitResult Hit;
    bool bHit = Pawn->GetWorld()->SweepSingleByChannel(
        Hit,
        TraceStart,
        TraceEnd,
        FQuat::Identity,
        ECC_WorldStatic,
        FCollisionShape::MakeSphere(ObstacleSphereRadius),
        Params
    );

    if (bHit)
    {
        // 장애물 법선에서 횡회전 방향 계산
        FVector AvoidDir = FVector::CrossProduct(Hit.Normal, FVector::UpVector).GetSafeNormal();
        FVector AvoidLocation = PawnLoc + AvoidDir * AvoidanceRadius;

        AICon->MoveToLocation(
            AvoidLocation,
            AcceptanceRadius,
            true, true, false, false, 0, true
        );
    }
    else
    {
        FVector TargetLoc = OwnerComp
            .GetBlackboardComponent()
            ->GetValueAsVector(TEXT("TargetLocation"));

        AICon->MoveToLocation(
            TargetLoc,
            AcceptanceRadius,
            true, true, false, false, 0, true
        );
    }

    EPathFollowingStatus::Type Status =
        AICon->GetPathFollowingComponent()->GetStatus();

    if (Status != EPathFollowingStatus::Moving)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
    }
}

EBTNodeResult::Type UBTTask_BossMoveToTarget::AbortTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    if (AAIController* AICon = OwnerComp.GetAIOwner())
    {
        AICon->StopMovement();
    }
    return EBTNodeResult::Aborted;
}