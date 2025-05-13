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

    // ���� ĳ���� ���� ȹ��
    MonsterCharacter = Cast<ABaseMonsterCharacter>(InPawn);
    UE_LOG(LogTemp, Warning, TEXT("1"));
    // ������ �� �����̺�� Ʈ�� ����
    if (BlackboardData && BlackboardComponent)
    {
        UE_LOG(LogTemp, Warning, TEXT("2"));
        BlackboardComponent->InitializeBlackboard(*(BlackboardData));
    }

    if (BehaviorTree && BlackboardComponent)
    {
        // �ʱ� ���¸� ������ ����
        SetPatrolling();
        UE_LOG(LogTemp, Warning, TEXT("3"));
        // �����̺�� Ʈ�� ����
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
        // ���¸� ������ ���� (0 = ����)
        BlackboardComponent->SetValueAsInt(StateKeyName, 0);
    }
}

void ABaseAIController::SetChasing(AActor* Target)
{
    if (BlackboardComponent)
    {
        // ��� ����
        BlackboardComponent->SetValueAsObject(TargetActorKeyName, Target);

        // ���¸� �������� ���� (1 = ����)
        BlackboardComponent->SetValueAsInt(StateKeyName, 1);

        // �̵� �ִϸ��̼� ���
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
        // ���¸� �������� ���� (2 = ����)
        BlackboardComponent->SetValueAsInt(StateKeyName, 2);

        // ���� ����
        MonsterCharacter->PerformAttack();
    }
}

