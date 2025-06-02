#include "AI/Task/BTTask_TeleportToLocation.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Pawn.h"

UBTTask_TeleportToLocation::UBTTask_TeleportToLocation()
{
    NodeName = TEXT("Teleport To Location");
    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_TeleportToLocation::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    // 블랙보드에서 목적지 벡터 꺼내기
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return EBTNodeResult::Failed;
    const FVector Dest = BB->GetValueAsVector(DestinationKey.SelectedKeyName);

    // AIController & Pawn 확인
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon) return EBTNodeResult::Failed;
    APawn* Pawn = AICon->GetPawn();
    if (!Pawn) return EBTNodeResult::Failed;

    // **디버그 로그**: Dest 와 현재 위치 비교
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] Dest=%s, Current=%s"),
        *Dest.ToString(), *Pawn->GetActorLocation().ToString());

    // 1) 텔레포트 시도 (TeleportPhysics 대신 TeleportTo 사용)
    const FRotator CurrentRot = Pawn->GetActorRotation();
    const bool bSuccess = Pawn->TeleportTo(Dest, CurrentRot, false, true);
    UE_LOG(LogTemp, Warning, TEXT("[Teleport] TeleportTo returned %s"),
        bSuccess ? TEXT("true") : TEXT("false"));

    return bSuccess ? EBTNodeResult::Succeeded : EBTNodeResult::Failed;
}