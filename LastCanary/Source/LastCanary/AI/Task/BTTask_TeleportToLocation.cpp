#include "AI/Task/BTTask_TeleportToLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "NavigationSystem.h"

UBTTask_TeleportToLocation::UBTTask_TeleportToLocation()
{
    NodeName = TEXT("Teleport To Location");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_TeleportToLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
        return EBTNodeResult::Failed;

    FVector RawDest = BB->GetValueAsVector(DestinationKey.SelectedKeyName);

    // 유효성 검사
    const float InvalidThreshold = 1e30f;
    if (!FMath::IsFinite(RawDest.X) || !FMath::IsFinite(RawDest.Y) || !FMath::IsFinite(RawDest.Z)
        || FMath::Abs(RawDest.X) > InvalidThreshold
        || FMath::Abs(RawDest.Y) > InvalidThreshold
        || FMath::Abs(RawDest.Z) > InvalidThreshold)
    {
        UE_LOG(LogTemp, Warning, TEXT("[Teleport] RawDest가 유효하지 않습니다: %s"), *RawDest.ToString());
        return EBTNodeResult::Failed;
    }

    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)
        return EBTNodeResult::Failed;

    UWorld* World = Pawn->GetWorld();
    if (!World)
        return EBTNodeResult::Failed;

    FVector AdjustedDest = RawDest;

    // 1) NavMesh 프로젝션 시도
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
    {
        FNavLocation ProjectedLoc;
        const FVector Extents(500.f, 500.f, 2000.f);
        if (NavSys->ProjectPointToNavigation(RawDest, ProjectedLoc, Extents))
        {
            AdjustedDest = ProjectedLoc.Location;
            UE_LOG(LogTemp, Warning, TEXT("[Teleport] Projection 성공: %s"), *AdjustedDest.ToString());
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Teleport] Projection 실패, RawDest 사용: %s"), *RawDest.ToString());
        }
    }

    // 2) LineTrace로 바닥 높이(Z) 보정
    {
        const float TraceHeightAbove = 1000.f;
        const float TraceDepthBelow = 1000.f;
        FVector TraceStart = FVector(AdjustedDest.X, AdjustedDest.Y, AdjustedDest.Z + TraceHeightAbove);
        FVector TraceEnd = FVector(AdjustedDest.X, AdjustedDest.Y, AdjustedDest.Z - TraceDepthBelow);

        FHitResult Hit;
        FCollisionQueryParams Params(NAME_None, false, Pawn);
        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, Params))
        {
            AdjustedDest.Z = Hit.Location.Z;
            UE_LOG(LogTemp, Warning, TEXT("[Teleport] LineTrace 성공 Z = %f"), AdjustedDest.Z);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("[Teleport] LineTrace 실패, Z 유지 = %f"), AdjustedDest.Z);
        }
    }

    // 3) 월드 바운드 내로 Clamp
    const float MaxBound = 20000.f;
    AdjustedDest.X = FMath::Clamp(AdjustedDest.X, -MaxBound, MaxBound);
    AdjustedDest.Y = FMath::Clamp(AdjustedDest.Y, -MaxBound, MaxBound);
    // Z는 이미 지형에 맞게 보정됨

    // (옵션) 디버그용 범위 시각화
    //DrawDebugSphere(World, AdjustedDest, 25.f, 8, FColor::Blue, false, 2.f);

    // 4) Collision 끄고 Teleport
    Pawn->SetActorEnableCollision(false);
    const FRotator CurrentRot = Pawn->GetActorRotation();
    bool bSuccess = Pawn->TeleportTo(AdjustedDest, CurrentRot, true, true);
    Pawn->SetActorEnableCollision(true);

    UE_LOG(LogTemp, Warning, TEXT("[Teleport] TeleportTo 결과 = %s"), bSuccess ? TEXT("성공") : TEXT("실패"));

    return bSuccess
        ? EBTNodeResult::Succeeded
        : EBTNodeResult::Failed;
}