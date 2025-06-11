#include "AI/BaseAIController.h"
#include "AI/BaseMonsterCharacter.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BehaviorTree/BehaviorTreeComponent.h"
#include "GameFramework/CharacterMovementComponent.h"

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

	MonsterCharacter = Cast<ABaseMonsterCharacter>(InPawn);

	if (BlackboardData && BlackboardComponent)
	{
		BlackboardComponent->InitializeBlackboard(*(BlackboardData));
	}

	if (BehaviorTree && BlackboardComponent)
	{
		BlackboardComponent->SetValueAsInt(StateKeyName, -1);

		GetWorld()->GetTimerManager().SetTimer(
			DissolveTimerHandle,
			this,
			&ABaseAIController::SetPatrolling,
			2.0f,
			false
		);

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
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
	//순찰
	if (BlackboardComponent && MyCharacter)
	{
		UCharacterMovementComponent* MovementComp = MyCharacter->GetCharacterMovement();
		if (MovementComp)
		{
			MovementComp->MaxWalkSpeed = 200.f;
		}

		BlackboardComponent->SetValueAsInt(StateKeyName, 0);
	}
}

void ABaseAIController::SetChasing(AActor* Target)
{
	ACharacter* MyCharacter = Cast<ACharacter>(GetPawn());
	//추격
	if (BlackboardComponent&& MyCharacter)
	{
		UCharacterMovementComponent* MovementComp = MyCharacter->GetCharacterMovement();
		if (MovementComp)
		{
			MovementComp->MaxWalkSpeed = 300.f;
		}

		BlackboardComponent->SetValueAsObject(TargetActorKeyName, Target);
		
		BlackboardComponent->SetValueAsInt(StateKeyName, 1);

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
		//공격
		BlackboardComponent->SetValueAsInt(StateKeyName, 2);

		MonsterCharacter->PerformAttack();
	}
}

void ABaseAIController::SetDeath()
{
	BlackboardComponent->SetValueAsInt(StateKeyName, 4);
}