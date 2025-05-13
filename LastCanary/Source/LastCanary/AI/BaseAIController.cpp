#include "AI/BaseAIController.h"
#include "AI/BaseMonsterCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"

ABaseAIController::ABaseAIController()
{
    BehaviorTreeComponent = CreateDefaultSubobject<UBehaviorTreeComponent>(TEXT("BehaviorTreeComponent"));
    BlackboardComponent = CreateDefaultSubobject<UBlackboardComponent>(TEXT("BlackboardComponent"));
}

void ABaseAIController::BeginPlay()
{
    Super::BeginPlay();
}

void ABaseAIController::OnPossess(APawn* InPawn)
{
    Super::OnPossess(InPawn);

    // 몬스터 캐릭터 참조 획득
    MonsterCharacter = Cast<ABaseMonsterCharacter>(InPawn);
    UE_LOG(LogTemp, Warning, TEXT("1"));
    // 블랙보드 및 비헤이비어 트리 시작
    if (BlackboardData && BlackboardComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("2"));
        BlackboardComponent->InitializeBlackboard(*(BlackboardData));
    }

    if (BehaviorTree && BlackboardComponent)
    {
        // 초기 상태를 순찰로 설정
        SetPatrolling();
        UE_LOG(LogTemp, Warning, TEXT("3"));
        // 비헤이비어 트리 실행
        RunBehaviorTree(BehaviorTree);
    }
}

void ABaseAIController::OnUnPossess()
{
    Super::OnUnPossess();

    if (BehaviorTreeComponent)
    {
        BehaviorTreeComponent->StopTree();
    }
}

void ABaseAIController::SetPatrolling()
{
    if (BlackboardComponent)
    {
        // 상태를 순찰로 설정 (0 = 순찰)
        BlackboardComponent->SetValueAsInt(StateKeyName, 0);
    }
}

void ABaseAIController::SetChasing(AActor* Target)
{
    if (BlackboardComponent)
    {
        // 대상 설정
        BlackboardComponent->SetValueAsObject(TargetActorKeyName, Target);

        // 상태를 추적으로 설정 (1 = 추적)
        BlackboardComponent->SetValueAsInt(StateKeyName, 1);

        // 이동 애니메이션 재생
        if (MonsterCharacter)
        {
            MonsterCharacter->MulticastAIMove();
        }
    }
}

void ABaseAIController::SetAttacking()
{
    if (BlackboardComponent && MonsterCharacter)
    {
        // 상태를 공격으로 설정 (2 = 공격)
        BlackboardComponent->SetValueAsInt(StateKeyName, 2);

        // 공격 수행
        MonsterCharacter->PerformAttack();
    }
}

