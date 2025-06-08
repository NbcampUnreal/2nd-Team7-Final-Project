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
    // 1) 블랙보드에서 Dest 읽기
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB)
    {
        return EBTNodeResult::Failed;
    }

    FVector RawDest = BB->GetValueAsVector(DestinationKey.SelectedKeyName);

    // 1-1) Dest가 초기화되지 않은 쓰레기 값인지 검사 (FLT_MAX 급 값)
    const float InvalidThreshold = 1e30f;
    if (!FMath::IsFinite(RawDest.X) || !FMath::IsFinite(RawDest.Y) || !FMath::IsFinite(RawDest.Z)
        || FMath::Abs(RawDest.X) > InvalidThreshold
        || FMath::Abs(RawDest.Y) > InvalidThreshold
        || FMath::Abs(RawDest.Z) > InvalidThreshold
        )
    {
        UE_LOG(LogTemp, Warning, TEXT("[Teleport] RawDest가 유효하지 않습니다. Dest=%s"), *RawDest.ToString());
        return EBTNodeResult::Failed;
    }

    // 2) AIController & Pawn 가져오기
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
    {
        return EBTNodeResult::Failed;
    }

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)
    {
        return EBTNodeResult::Failed;
    }

    UE_LOG(LogTemp, Warning, TEXT("[Teleport] RawDest = %s, Current = %s"),
        *RawDest.ToString(), *Pawn->GetActorLocation().ToString());

    // 3) NavMesh 위 유효 좌표로 보정 시도 (ProjectPointToNavigation)
    FVector AdjustedDest = RawDest;
    if (UWorld* World = Pawn->GetWorld())
    {
        if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
        {
            FNavLocation ProjectedLoc;
            const FVector Extents(500.0f, 500.0f, 2000.0f);
            if (NavSys->ProjectPointToNavigation(RawDest, ProjectedLoc, Extents))
            {
                AdjustedDest = ProjectedLoc.Location;
                UE_LOG(LogTemp, Warning, TEXT("[Teleport] Projection 성공 → AdjustedDest = %s"), *AdjustedDest.ToString());
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("[Teleport] Projection 실패, RawDest 사용 = %s"), *RawDest.ToString());
            }
        }
    }
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] After Nav Correction (또는 RawDest) = %s"), *AdjustedDest.ToString());

    // 4) 월드 바운드(±20000) 내에 있도록 Clamp
    const float MaxBound = 20000.0f;
    AdjustedDest.X = FMath::Clamp(AdjustedDest.X, -MaxBound, MaxBound);
    AdjustedDest.Y = FMath::Clamp(AdjustedDest.Y, -MaxBound, MaxBound);
    // Z 범위는 레벨에 맞춰 필요 시 조정
    AdjustedDest.Z = FMath::Clamp(AdjustedDest.Z, -500.0f, 3000.0f);
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] After Clamping = %s"), *AdjustedDest.ToString());

    // 5) Collision 비활성화 (필요 시)
    Pawn->SetActorEnableCollision(false);

    // 6) TeleportTo 시도 (충돌 검사 없이 강제 이동)
    const FRotator CurrentRot = Pawn->GetActorRotation();
    bool bSuccess = Pawn->TeleportTo(AdjustedDest, CurrentRot, true, true);
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] TeleportTo returned %s"),
        bSuccess ? TEXT("true") : TEXT("false"));

    // 7) Teleport 후 Collision 복원
    Pawn->SetActorEnableCollision(true);

    return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}