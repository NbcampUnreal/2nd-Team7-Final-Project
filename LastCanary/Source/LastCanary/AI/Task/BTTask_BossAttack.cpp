#include "AI/Task/BTTask_BossAttack.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "AI/BaseAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_BossAttack::UBTTask_BossAttack(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Boss Attack");
}

EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    AAIController* AICon = OwnerComp.GetAIOwner();
    if (!AICon)
        return EBTNodeResult::Failed;

    APawn* Pawn = AICon->GetPawn();
    if (!Pawn)
        return EBTNodeResult::Failed;

    ABaseBossMonsterCharacter* Boss = Cast<ABaseBossMonsterCharacter>(Pawn);
    if (!Boss)
        return EBTNodeResult::Failed;

    UBlackboardComponent* BB = OwnerComp.GetBlackboardComponent();
    float TargetDistance = BB->GetValueAsFloat("TargetDistance");

    // RequestAttack(): 쿨다운·조건 체크 후 공격 몽타주 실행
    const bool bAttacked = Boss->RequestAttack(TargetDistance);
    return bAttacked
        ? EBTNodeResult::Succeeded
        : EBTNodeResult::Failed;
}
