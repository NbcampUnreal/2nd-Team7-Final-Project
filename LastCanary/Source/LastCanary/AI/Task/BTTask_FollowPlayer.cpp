#include "AI/Task/BTTask_FollowPlayer.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Navigation/PathFollowingComponent.h"

UBTTask_FollowPlayer::UBTTask_FollowPlayer()
{
    NodeName = TEXT("Follow Player");
}

EBTNodeResult::Type UBTTask_FollowPlayer::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    UObject* Obj = BB->GetValueAsObject(TEXT("TargetActor"));
    AActor* Target = Cast<AActor>(Obj);
    AAIController* AICon = OwnerComp.GetAIOwner();

    if (!Target || !AICon)
        return EBTNodeResult::Failed;

    FAIMoveRequest MoveReq;
    MoveReq.SetGoalActor(Target);
    MoveReq.SetAcceptanceRadius(AcceptanceRadius);
    MoveReq.SetUsePathfinding(true);

    FNavPathSharedPtr NavPath;
    AICon->MoveTo(MoveReq, &NavPath);

    return EBTNodeResult::Succeeded;
}
