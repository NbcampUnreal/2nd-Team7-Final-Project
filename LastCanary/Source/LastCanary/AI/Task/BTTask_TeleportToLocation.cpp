#include "AI/Task/BTTask_TeleportToLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"
#include "Components/CapsuleComponent.h"
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
    if (!BB) return EBTNodeResult::Failed;

    FVector RawDest = BB->GetValueAsVector(DestinationKey.SelectedKeyName);
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;
    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)  return EBTNodeResult::Failed;
    UWorld* World = Pawn->GetWorld();
    if (!World) return EBTNodeResult::Failed;

    // 1) NavMesh 프로젝션
    FVector AdjustedDest = RawDest;
    if (UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(World))
    {
        FNavLocation NavLoc;
        if (NavSys->ProjectPointToNavigation(RawDest, NavLoc, FVector(500, 500, 2000)))
            AdjustedDest = NavLoc.Location;
    }

    // 2) LineTrace로 Z 보정
    {
        FVector Start = AdjustedDest + FVector(0, 0, 1000);
        FVector End = AdjustedDest - FVector(0, 0, 1000);
        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility))
            AdjustedDest.Z = Hit.Location.Z;
    }

    // 3) 캡슐 반높이만큼 올려 주기
    if (UCapsuleComponent* Cap = Pawn->FindComponentByClass<UCapsuleComponent>())
    {
        AdjustedDest.Z += Cap->GetScaledCapsuleHalfHeight();
    }

    // (옵션) 디버그
    DrawDebugSphere(World, AdjustedDest, 25.f, 8, FColor::Blue, false, 2.f);

    // 4) Collision 끄고 Teleport
    Pawn->SetActorEnableCollision(false);
    bool bOK = Pawn->TeleportTo(AdjustedDest, Pawn->GetActorRotation(), false, true);
    Pawn->SetActorEnableCollision(true);

    return bOK ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}