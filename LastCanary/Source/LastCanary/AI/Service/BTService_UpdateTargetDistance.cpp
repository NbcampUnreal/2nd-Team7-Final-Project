#include "AI/Service/BTService_UpdateTargetDistance.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UBTService_UpdateTargetDistance::UBTService_UpdateTargetDistance()
{
    NodeName = TEXT("Update Target Distance");
    Interval = 0.2f;
}

void UBTService_UpdateTargetDistance::TickNode(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds
)
{
    Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!BB || !AICon) return;

    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    APawn* Pawn = AICon->GetPawn();
    if (!Target || !Pawn || !Target->IsValidLowLevel())
    {
        BB->ClearValue(TEXT("TargetActor"));
        BB->SetValueAsFloat(TEXT("TargetDistance"), 0.f);
        return;
    }

    const float Dist = FVector::Distance(
        Pawn->GetActorLocation(),
        Target->GetActorLocation()
    );
    BB->SetValueAsFloat(TEXT("TargetDistance"), Dist);
}
