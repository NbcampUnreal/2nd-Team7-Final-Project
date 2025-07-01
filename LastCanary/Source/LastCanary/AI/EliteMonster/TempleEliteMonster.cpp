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
#include "Item/ResourceNode.h"
#include "Engine/DamageEvents.h"

ATempleEliteMonster::ATempleEliteMonster()
{
	Extra_AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Extra_AttackCollider"));
	Extra_AttackCollider->SetupAttachment(RootComponent);
	Extra_AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Extra_AttackCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ABaseMonsterCharacter::OnAttackHit);
}

float ATempleEliteMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

			if (PointDamageEvent->HitInfo.BoneName == "head")
			{
				NeckHitCount++;
				UE_LOG(LogTemp, Error, TEXT("카운트 = %d"), NeckHitCount);

				if (NeckHitCount >= 10) //카운트 횟수 에디터로 뺄 것, 쿨타임 넣을 것
				{
					NeckHitCount = 0;
					if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
					{
						AIController->SetStun(GroggyTime);
					}
				}
			}
		}
		return 0;
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
			else if (Stimulus.Tag.IsEqual(FName("Box")))
			{
				if (!OnReceiveTracking)
				{
					if (AResourceNode* ResourceNode = Cast<AResourceNode>(Actor))
					{
						BlackboardComp->SetValueAsVector(FName("BoxVector"), Stimulus.StimulusLocation);
						AIController->SetSearching();
					}
				}
			}
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

void ATempleEliteMonster::ToggleOnReceive()
{
	if (!HasAuthority()) return;

	OnReceiveTracking = !OnReceiveTracking;
}