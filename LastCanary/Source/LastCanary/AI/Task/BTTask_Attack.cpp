#include "AI/Task/BTTask_Attack.h"
#include "AI/BaseAIController.h"
#include "AI/BaseMonsterCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "GameFramework/Character.h"

UBTTask_Attack::UBTTask_Attack()
{
    NodeName = TEXT("Attack");

    bCreateNodeInstance = true;
}

EBTNodeResult::Type UBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
    ABaseAIController* AIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (!AIController)
    {
        return EBTNodeResult::Failed;
    }

    ABaseMonsterCharacter* Monster = Cast<ABaseMonsterCharacter>(AIController->GetPawn());
    if (!Monster)
    {
        return EBTNodeResult::Failed;
    }

    Monster->PerformAttack();

    ABaseAIController* BaseAIController = Cast<ABaseAIController>(OwnerComp.GetAIOwner());
    if (BaseAIController)
    {
        AActor* Target = Cast<AActor>(OwnerComp.GetBlackboardComponent()->GetValueAsObject("TargetActor"));
        BaseAIController->SetChasing(Target);
    }

    return EBTNodeResult::Succeeded;

}