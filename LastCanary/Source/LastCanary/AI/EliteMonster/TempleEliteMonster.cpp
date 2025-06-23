#include "AI/EliteMonster/TempleEliteMonster.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ATempleEliteMonster::ATempleEliteMonster()
{
    GetCharacterMovement()->MaxWalkSpeed = 200.0f;
}

void ATempleEliteMonster::ReceiveTrackingTarget(AActor* NewTarget)
{
	if (!HasAuthority()) return;

	if (ABaseAIController* AI = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BB = AI->GetBlackboardComponent())
		{
			BB->SetValueAsObject("TargetActor", NewTarget);
			AI->SetChasing(NewTarget); // 상태 변경
		}
	}
}
