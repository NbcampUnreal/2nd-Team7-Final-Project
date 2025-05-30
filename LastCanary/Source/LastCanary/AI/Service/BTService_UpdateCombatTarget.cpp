#include "AI/Service/BTService_UpdateCombatTarget.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdateCombatTarget::UBTService_UpdateCombatTarget()
{
    NodeName = TEXT("Update Combat Move Target");
    Interval = 0.1f;
    bCreateNodeInstance = true;
}

void UBTService_UpdateCombatTarget::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    if (!BB) return;

    UObject* Obj = BB->GetValueAsObject(TargetActorKey.SelectedKeyName);
    if (AActor* Target = Cast<AActor>(Obj))
    {
        // TargetActor가 있으면 그 위치를 TargetLocationKey에 기록
        BB->SetValueAsVector(
            TargetLocationKey.SelectedKeyName,
            Target->GetActorLocation()
        );
    }
    // TargetActor가 없으면 패스
}
