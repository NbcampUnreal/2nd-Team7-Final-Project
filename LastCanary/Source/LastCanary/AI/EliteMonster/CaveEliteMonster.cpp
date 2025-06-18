#include "AI/EliteMonster/CaveEliteMonster.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Perception/AISense_Hearing.h"
#include "AIController.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Character/BaseCharacter.h"

ACaveEliteMonster::ACaveEliteMonster()
{
	PrimaryActorTick.bCanEverTick = false;

	if (UCapsuleComponent* CapsuleComp = GetCapsuleComponent())
	{
		CapsuleComp->SetGenerateOverlapEvents(true);
		CapsuleComp->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	}
}

void ACaveEliteMonster::FreezeAI()
{
	if (bIsFrozen || CooldownTimerHandle.IsValid()) return;

	bIsFrozen = true;

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->StopMovementImmediately();
		MovementComp->SetMovementMode(MOVE_None);//moveto 무시
	}

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		if (UAIPerceptionComponent* PerceptionComp = BaseAIController->GetPerceptionComponent())
		{
			// 청각 완전 비활성화
			PerceptionComp->SetSenseEnabled(UAISense_Hearing::StaticClass(), false);

			PerceptionComp->SetActive(false);
		}
	}

	/*if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComp = BaseAIController->GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsObject("TargetActor", nullptr);
		}
	}*/


	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		BaseAIController->SetStop();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().SetTimer(
			FreezeTimerHandle,
			this,
			&ACaveEliteMonster::UnfreezeAI,
			MaxFreezeTime,
			false
		);
	}
}

void ACaveEliteMonster::UnfreezeAI()
{
	if (!bIsFrozen)
	{
		return;
	}

	bIsFrozen = false;

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		if (UAIPerceptionComponent* PerceptionComp = BaseAIController->GetPerceptionComponent())
		{
			PerceptionComp->SetSenseEnabled(UAISense_Hearing::StaticClass(), true);

			PerceptionComp->SetActive(true);
		}
	}

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComp = BaseAIController->GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsObject("TargetActor", nullptr);
		}
	}

	if (UCharacterMovementComponent* MovementComp = GetCharacterMovement())
	{
		MovementComp->SetMovementMode(MOVE_Walking);
	}

	if (ABaseAIController* BaseAIController = Cast<ABaseAIController>(GetController()))
	{
		BaseAIController->SetPatrolling();
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(FreezeTimerHandle);

		World->GetTimerManager().SetTimer(
			CooldownTimerHandle,
			this,
			&ACaveEliteMonster::CooldownEnd,
			FreezeCooldown,
			false
		);
	}
}

void ACaveEliteMonster::CooldownEnd()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(CooldownTimerHandle);
	}
}

void ACaveEliteMonster::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor) return;

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
							BlackboardComp->SetValueAsVector(FName("LastHeardLocation"), Stimulus.StimulusLocation);
						}
					}
				}

				if (UWorld* World = GetWorld())
				{
					World->GetTimerManager().ClearTimer(ForgetTargetTimerHandle);
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
							&ACaveEliteMonster::ForgetTarget,
							HearingMaxAge,
							false
						);
					}
				}
			}
		}
	}
}

void ACaveEliteMonster::ForgetTarget()
{
	if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
		{
			BlackboardComp->ClearValue(FName("TargetActor"));
			BlackboardComp->ClearValue(FName("LastHeardLocation"));
		}
	}

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ForgetTargetTimerHandle);
	}
}

//void ACaveEliteMonster::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
//{
//	if (!Actor) return;
//
//	if (Stimulus.WasSuccessfullySensed())
//	{
//		if (Stimulus.Tag.IsEqual(FName("Boss")))
//		{
//			UE_LOG(LogTemp, Warning, TEXT("Boss tag - IGNORING"));
//			return;
//		}
//
//		if (!Stimulus.Tag.IsEqual(FName("CaveMonster")))
//		{
//			return;
//		}
//	}
//	
//	if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
//	{
//		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
//		{
//			if (Stimulus.WasSuccessfullySensed())
//			{
//				if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Actor))
//				{
//					BlackboardComp->SetValueAsObject(FName("TargetActor"), BaseCharacter);
//				}
//
//				else if (AGunBase* GunBase = Cast<AGunBase>(Actor))
//				{
//					if (AActor* GunOwner = GunBase->GetOwner())
//					{
//						if (ABaseCharacter* GunOwnerCharacter = Cast<ABaseCharacter>(GunOwner))
//						{
//							BlackboardComp->SetValueAsObject(FName("TargetActor"), GunOwnerCharacter);
//							BlackboardComp->SetValueAsVector(FName("LastHeardLocation"), Stimulus.StimulusLocation);
//						}
//					}
//				}
//			}
//			else
//			{
//				if (!Stimulus.WasSuccessfullySensed())
//				{
//					UObject* CurrentTarget = BlackboardComp->GetValueAsObject(FName("TargetActor"));
//					if (CurrentTarget == Actor)
//					{
//						BlackboardComp->ClearValue(FName("TargetActor"));
//						BlackboardComp->ClearValue(FName("LastHeardLocation"));
//					}
//				}
//			}
//		}
//	}
//}
