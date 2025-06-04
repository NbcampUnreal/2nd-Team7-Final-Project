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

    // (1) 블랙보드에서 Dest 읽기
    FVector RawDest = BB->GetValueAsVector(DestinationKey.SelectedKeyName);

    // (1-1) Dest가 초기화되지 않아 쓰레기 값(±FLT_MAX 등)으로 남아 있는지 검사
    const float InvalidThreshold = 1e30f;
    if (!FMath::IsFinite(RawDest.X) || !FMath::IsFinite(RawDest.Y) || !FMath::IsFinite(RawDest.Z)
        || FMath::Abs(RawDest.X) > InvalidThreshold
        || FMath::Abs(RawDest.Y) > InvalidThreshold
        || FMath::Abs(RawDest.Z) > InvalidThreshold
        )
    {
        UE_LOG(LogTemp, Warning, TEXT("[Teleport] RawDest가 유효하지 않아 Teleport를 건너뜁니다. Dest=%s"), *RawDest.ToString());
        return EBTNodeResult::Failed;
    }

    // (2) AIController & Pawn 가져오기
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;
    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)
        return EBTNodeResult::Failed;

    UE_LOG(LogTemp, Warning, TEXT("[Teleport] RawDest = %s, Current = %s"),
        *RawDest.ToString(), *Pawn->GetActorLocation().ToString());

    // (3) NavMesh 위 유효 좌표로 보정 (ProjectPointToNavigation)
    FVector AdjustedDest = RawDest;
    if (UWorld* World = Pawn->GetWorld())
    {
        if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
        {
            FNavLocation ProjectedLoc;
            if (NavSys->ProjectPointToNavigation(RawDest, ProjectedLoc, FVector(200.f, 200.f, 200.f)))
            {
                AdjustedDest = ProjectedLoc.Location;
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[Teleport] Nav projection 실패! RawDest=%s"), *RawDest.ToString());
                // NavMesh 위로 보정 못 하면, 여기서도 빌드를 중단해도 무방합니다.
                return EBTNodeResult::Failed;
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] After Nav Correction = %s"), *AdjustedDest.ToString());

    // (4) 월드 바운드(±20000) 내에 있도록 Clamp
    const float MaxBound = 20000.f;
    AdjustedDest.X = FMath::Clamp(AdjustedDest.X, -MaxBound, MaxBound);
    AdjustedDest.Y = FMath::Clamp(AdjustedDest.Y, -MaxBound, MaxBound);
    AdjustedDest.Z = FMath::Clamp(AdjustedDest.Z, -500.f, 3000.f); // 레벨마다 알맞게 수정
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] After Clamping = %s"), *AdjustedDest.ToString());

    // (5) Collision 비활성화
    Pawn->SetActorEnableCollision(false);

    // (6) TeleportTo 시도
    const FRotator CurrentRot = Pawn->GetActorRotation();
    bool bSuccess = Pawn->TeleportTo(AdjustedDest, CurrentRot, true, true);
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] TeleportTo returned %s"),
        bSuccess ? TEXT("true") : TEXT("false"));

    // (7) Teleport 후 Collision 복원
    Pawn->SetActorEnableCollision(true);

    return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}