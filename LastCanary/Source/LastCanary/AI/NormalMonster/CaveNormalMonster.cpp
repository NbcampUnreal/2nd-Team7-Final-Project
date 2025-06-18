#include "AI/NormalMonster/CaveNormalMonster.h"
#include "Item/EquipmentItem/GunBase.h"
#include "AI/BaseAIController.h"
#include "Character/BaseCharacter.h"
#include "BehaviorTree/BlackboardComponent.h"

ACaveNormalMonster::ACaveNormalMonster()
{

}

void ACaveNormalMonster::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
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
                            &ACaveNormalMonster::ForgetTarget,
                            HearingMaxAge,
                            false
                        );
                    }
                }
            }
        }
    }
}

void ACaveNormalMonster::ForgetTarget()
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

//void ACaveNormalMonster::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
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