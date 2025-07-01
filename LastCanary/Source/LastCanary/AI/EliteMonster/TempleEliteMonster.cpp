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
#include "Containers/Map.h"

ATempleEliteMonster::ATempleEliteMonster()
{
	Extra_AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("Extra_AttackCollider"));
	Extra_AttackCollider->SetupAttachment(RootComponent);
	Extra_AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	Extra_AttackCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ABaseMonsterCharacter::OnAttackHit);
}

void ATempleEliteMonster::BeginPlay()
{
	Super::BeginPlay();

	if (BoneHitCountMultipliers.IsEmpty())
	{
		BoneHitCountMultipliers.Add(TEXT("head"), 2.5f);//머리
		BoneHitCountMultipliers.Add(TEXT("spine_02"), 3.0f);//허리
		BoneHitCountMultipliers.Add(TEXT("spine_03"), 1.5f);//몸통
		BoneHitCountMultipliers.Add(TEXT("pelvis"), 1.5f);

		//팔
		BoneHitCountMultipliers.Add(TEXT("upperarm_l"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("lowerarm_l"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("hand_l"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("upperarm_r"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("lowerarm_r"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("hand_r"), 1.0f);

		//다리
		BoneHitCountMultipliers.Add(TEXT("thigh_L"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("calf_l"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("foot_L"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("thigh_R"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("calf_r"), 1.0f);
		BoneHitCountMultipliers.Add(TEXT("foot_R"), 1.0f);
	}
}

float ATempleEliteMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (DamageAmount > 0.0f)
	{
		if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
		{
			const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);

			if (const float* Multiplier = BoneHitCountMultipliers.Find(PointDamageEvent->HitInfo.BoneName))
			{
				HitCount += *Multiplier;
				UE_LOG(LogTemp, Error, TEXT("현재 카운트 = %f (맞은 뼈: %s)"), HitCount, *PointDamageEvent->HitInfo.BoneName.ToString());
			}

			if (HitCount >= GroggyCount)
			{
				HitCount = 0.0f;

				if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
				{
					AIController->SetStun(GroggyTime);
				}
			}
		}
	}

	return 0.0f;
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