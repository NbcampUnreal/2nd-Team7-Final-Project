#include "AI/NormalMonster/WarderBaseMonster.h"
#include "AI/BaseAIController.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
//#include "Kismet/GameplayStatics.h"
#include "AI/NormalMonster/CaveNormalMonster.h"
#include "Item/EquipmentItem/GunBase.h"
#include "Character/BaseCharacter.h"
#include "Item/ResourceNode.h"

AWarderBaseMonster::AWarderBaseMonster()
{
	MaxHP = 200.0f;
	//CurrentHP = MaxHP;
	GetCharacterMovement()->MaxWalkSpeed = 300.f;

	AttackDamage = 20.0f;
}

void AWarderBaseMonster::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
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
                if (AResourceNode* ResourceNode = Cast<AResourceNode>(Actor))
                {
                    BlackboardComp->SetValueAsVector(FName("BoxVector"), Stimulus.StimulusLocation);
                    AIController->SetSearching();
                    if (UCharacterMovementComponent* Movement = GetCharacterMovement())
                    {
                        Movement->MaxWalkSpeed += 50.0f;
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
                            &AWarderBaseMonster::ForgetTarget,
                            HearingMaxAge,
                            false
                        );
                    }
                }
            }
        }
    }
}

void AWarderBaseMonster::ForgetTarget()
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

void AWarderBaseMonster::WarderGimmick()
{

}