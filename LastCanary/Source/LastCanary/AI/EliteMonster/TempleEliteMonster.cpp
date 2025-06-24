#include "AI/EliteMonster/TempleEliteMonster.h"
#include "Item/EquipmentItem/GunBase.h"
#include "AI/BaseAIController.h"
#include "Character/BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AI/BaseAIController.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"

ATempleEliteMonster::ATempleEliteMonster()
{
	Extra_AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Extra_AttackCollider"));
	Extra_AttackCollider->SetupAttachment(RootComponent);
	Extra_AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Extra_AttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABaseMonsterCharacter::OnAttackHit);
}

void ATempleEliteMonster::EnableAttackCollider()
{
	Super::EnableAttackCollider();

	Extra_AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void ATempleEliteMonster::DisableAttackCollider()
{
	Super::DisableAttackCollider();

	Extra_AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ATempleEliteMonster::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

	Super::HandlePerceptionUpdate(Actor, Stimulus);

	if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
		{
			if (Stimulus.WasSuccessfullySensed() && Stimulus.Tag.IsEqual(FName("CaveMonster")))
			{

				if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Actor))
				{
					BlackboardComp->SetValueAsObject(FName("TargetActor"), BaseCharacter);
				}

				else if (AGunBase* GunBase = Cast<AGunBase>(Actor))
				{
					if (AActor* GunOwner = GunBase->GetOwner())
					{
						if (ABaseCharacter* GunOwnerCharacter = Cast<ABaseCharacter>(GunOwner))
						{
							BlackboardComp->SetValueAsObject(FName("TargetActor"), GunOwnerCharacter);
						}
					}
				}

				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().ClearTimer(ForgetTargetTimerHandle);
				}
			}
			/*else if (Stimulus.Tag.IsEqual(FName("Box")))
			{
				if (ABoxItem* BoxItem = Cast<ABoxItem>(Actor))
				{
					BlackboardComp->SetValueAsVector(FName("BoxVector"), Stimulus.StimulusLocation);

					AIController->SetSearching();
				}
			}*/
			else
			{
				if (UWorld* World = GetWorld())
				{
					if (!ForgetTargetTimerHandle.IsValid())
					{
						World->GetTimerManager().SetTimer(
							ForgetTargetTimerHandle,
							this,
							&ATempleEliteMonster::ForgetTarget,
							HearingMaxAge,
							false
						);
					}
				}
			}
		}
	}
}

void ATempleEliteMonster::ForgetTarget()
{
	if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
		{
			BlackboardComp->ClearValue(FName("TargetActor"));
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ForgetTargetTimerHandle);
	}
}
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
