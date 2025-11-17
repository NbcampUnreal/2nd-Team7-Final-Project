#include "AI/Task/BTTask_BossAttack.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "AI/BaseAIController.h"
#include "AIController.h"
#include "BehaviorTree/BlackboardComponent.h"

UBTTask_BossAttack::UBTTask_BossAttack(const FObjectInitializer& ObjInit)
    : Super(ObjInit)
{
    NodeName = TEXT("Boss Attack");
    bNotifyTick = true;
    bNotifyTaskFinished = true;
}

EBTNodeResult::Type UBTTask_BossAttack::ExecuteTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory)
{
    auto* AICon = OwnerComp.GetAIOwner();
    auto* Boss = Cast<ABaseBossMonsterCharacter>(AICon->GetPawn());
    auto* BB = OwnerComp.GetBlackboardComponent();
    float Dist = BB->GetValueAsFloat(TEXT("TargetDistance"));
    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    if (!AICon || !Boss || !Target)
        return EBTNodeResult::Failed;

    // 1) 한 번만 스킬 선택
    Boss->RequestAttack(Dist);

    // 2) 사거리 밖이면 이동
    float Range = Boss->GetNextAttackRange();
    float ApproachOffset = 50.f;                // 이만큼 더 안쪽으로 들어갑니다
    float StopDistance = FMath::Max(Range - ApproachOffset, 0.f);
    if (Dist > Range)
    {
        FVector Dir = (Target->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal();
        FVector Stop = Target->GetActorLocation() - Dir * StopDistance;
        AICon->MoveToLocation(Stop, /*Acceptance=*/10.f);
    }
    // 사거리 안이면 TickTask 에서 실행
    return EBTNodeResult::InProgress;
}

void UBTTask_BossAttack::TickTask(
    UBehaviorTreeComponent& OwnerComp,
    uint8* NodeMemory,
    float DeltaSeconds)
{
    auto* AICon = OwnerComp.GetAIOwner();
    auto* Boss = Cast<ABaseBossMonsterCharacter>(AICon->GetPawn());
    auto* BB = OwnerComp.GetBlackboardComponent();
    float Dist = BB->GetValueAsFloat(TEXT("TargetDistance"));
    AActor* Target = Cast<AActor>(BB->GetValueAsObject(TEXT("TargetActor")));
    if (!AICon || !Boss || !Target)
    {
        FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
        return;
    }

    // 1) 사거리 밖이면 계속 이동
    float Range = Boss->GetNextAttackRange();
    float ApproachOffset = 50.f;                // 이만큼 더 안쪽으로 들어갑니다
    float StopDistance = FMath::Max(Range - ApproachOffset, 0.f);
    if (Dist > Range)
    {
        FVector Dir = (Target->GetActorLocation() - Boss->GetActorLocation()).GetSafeNormal();
        FVector Stop = Target->GetActorLocation() - Dir * StopDistance;
        AICon->MoveToLocation(Stop, /*Acceptance=*/10.f);
        return;
    }

    // 2) 사거리 안이면 한 번만 공격
    AICon->StopMovement();
    Boss->ExecuteSelectedAttack();
    FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}