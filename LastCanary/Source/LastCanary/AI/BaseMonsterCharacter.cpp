#include "AI/BaseMonsterCharacter.h"
#include "AI/BaseAIController.h"
#include "Character/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Touch.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

#include "Character/BasePlayerState.h"

ABaseMonsterCharacter::ABaseMonsterCharacter()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	MaxHP = 100.f;
	CurrentHP = MaxHP;
	bIsDead = false;

	AttackDamage = 10.0f;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

	AIControllerClass = ABaseAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("AttackCollider"));
	AttackCollider->SetupAttachment(RootComponent);//GetMesh(), FName("hand_r"));
	//AttackCollider->SetRelativeLocation(FVector(100, 0, 0));
	AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	AttackCollider->OnComponentBeginOverlap.AddUniqueDynamic(this, &ABaseMonsterCharacter::OnAttackHit);

	UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
	if (CapsuleComp)
	{
		CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
	}

	SetReplicateMovement(true);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = 200.f;

	/*NavGenerationradius = 200.0f;
	NavRemovalradius = 300.0f;

	NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
	NavInvoker->SetGenerationRadii(NavGenerationradius, NavRemovalradius);*/

	GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy")));
	if (GetMesh())
	{
		GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	}
}

void ABaseMonsterCharacter::BeginPlay()
{
	Super::BeginPlay();
	InitializeBoneDamageMap();
	EnableStencilForAllMeshes(1);

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.AddUniqueDynamic(
			this, &ABaseMonsterCharacter::OnTargetPerceptionUpdated
		);
	}
}

void ABaseMonsterCharacter::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	StopAllCurrentActions();

	GetWorld()->GetTimerManager().ClearAllTimersForObject(this);

	if (AIPerceptionComponent)
	{
		AIPerceptionComponent->OnTargetPerceptionUpdated.RemoveAll(this);
		AIPerceptionComponent->SetActive(false);
	}
	if (ABaseAIController* AI = Cast<ABaseAIController>(GetController()))
	{
		AI->UnPossess();
		AI->Destroy();
	}

	Super::EndPlay(EndPlayReason);
}

void ABaseMonsterCharacter::InitializeBoneDamageMap()
{
	//cave
	BoneDamageMultipliers.Add("Neck", HeadShotMultiplier);
	BoneDamageMultipliers.Add("head", HeadShotMultiplier);
}

void ABaseMonsterCharacter::OnTargetPerceptionUpdated(AActor* Actor, FAIStimulus Stimulus)
{
	HandlePerceptionUpdate(Actor, Stimulus);
}

void ABaseMonsterCharacter::HandlePerceptionUpdate(AActor* Actor, FAIStimulus Stimulus)
{
	if (!Actor || !IsValid(this) || bIsDead) return;

	if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
	{
		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
		{
			if (Stimulus.WasSuccessfullySensed())
			{
				GetWorldTimerManager().ClearTimer(ForgetTargetTimer);

				if (ABaseCharacter* BaseCharacter = Cast<ABaseCharacter>(Actor))
				{
					BlackboardComp->SetValueAsObject(FName("TargetActor"), BaseCharacter);
				}
			}
			else
			{
				UObject* CurrentTarget = BlackboardComp->GetValueAsObject(FName("TargetActor"));
				if (CurrentTarget == Actor)
				{
					TWeakObjectPtr<ABaseMonsterCharacter> WeakThis = this;
					GetWorldTimerManager().SetTimer(ForgetTargetTimer,
						[WeakThis, BlackboardComp]()
					{
						if (WeakThis.IsValid() && IsValid(BlackboardComp))
						{
							BlackboardComp->ClearValue(FName("TargetActor"));
						}
					},
						3.0f, false);
				}
			}
		}
	}
}

float ABaseMonsterCharacter::GetDamageMultiplierForBone(FName BoneName)
{
	if (float* Multiplier = BoneDamageMultipliers.Find(BoneName))
	{
		return *Multiplier;
	}
	return 1.0f; // 기본값
}

float ABaseMonsterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority()) return 0.0f;
	if (!IsValid(DamageCauser)) return 0.0f;
	if (bIsDead) return 0.0f;

	//무적(엘리트)
	if (MaxHP <= 0)
	{
		return 0.0f;
	}

	float StunTime = 0.1f;
	float FinalDamage = DamageAmount;
	if (DamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		const FPointDamageEvent* PointDamageEvent = static_cast<const FPointDamageEvent*>(&DamageEvent);
		FName HitBoneName = PointDamageEvent->HitInfo.BoneName;

		float DamageMultiplier = GetDamageMultiplierForBone(HitBoneName);
		FinalDamage *= DamageMultiplier;
		if (HitBoneName == "head" || HitBoneName == "Neck")
		{
			StunTime = GroggyTime;
		}
	}

	float DamageApplied = FMath::Clamp(DamageAmount, 0.0f, CurrentHP);
	CurrentHP -= FinalDamage;//DamageAmount;


	if (CurrentHP > 0)
	{
		if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
		{
			AIController->SetStun(StunTime);//경직 시간
		}
	}
	else
	{
		CurrentHP = 0;
		bIsDead = true;

		StopAllCurrentActions();//몽타주 올 스탑

		GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		GetCharacterMovement()->SetMovementMode(MOVE_None);

		MulticastAIDeath();

		if (OnMonsterDeath.IsBound())
		{
			OnMonsterDeath.Broadcast(this);
		}

		if (HasAuthority())
		{
			GetWorldTimerManager().SetTimer(DeathTimerHandle, this,
				&ABaseMonsterCharacter::DestroyActor, 1.9f, false);
		}

		if (EventInstigator && EventInstigator->PlayerState)
		{
			if (ABasePlayerState* PS = Cast<ABasePlayerState>(EventInstigator->PlayerState))
			{
				PS->KillCount++; // 또는 PS->KillCount++; 등
			}
		}

	}


	return DamageApplied;
}

//공격 함수
void ABaseMonsterCharacter::OnAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;
	if (!OtherActor || OtherActor == this) return;
	if (!bIsAttacking) return;

	if (ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(OtherActor))
	{
		FDamageEvent DamageEvent;
		HitCharacter->TakeDamage(AttackDamage, DamageEvent, GetController(), this);

		DisableAttackCollider();
	}
}

void ABaseMonsterCharacter::StopAllCurrentActions()
{
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->StopAllMontages(0.2f);//0.2초에 걸쳐 부드럽게 정지
	}

	GetWorldTimerManager().ClearTimer(AttackTimerHandle);
	GetWorldTimerManager().ClearTimer(AttackEnableTimerHandle);
	GetWorldTimerManager().ClearTimer(ForgetTargetTimer);
	GetWorldTimerManager().ClearTimer(DeathTimerHandle);

	bIsAttacking = false;
	DisableAttackCollider();

	if (ABaseAIController* AIController = Cast<ABaseAIController>(GetController()))
	{
		if (ABaseAIController* AI = Cast<ABaseAIController>(GetController()))
		{
			AI->UnPossess();
			AI->Destroy();
		}
		if (UAIPerceptionComponent* PerceptionComp = AIController->GetPerceptionComponent())
		{
			PerceptionComp->OnTargetPerceptionUpdated.RemoveAll(this);
			PerceptionComp->SetActive(false);
		}
		if (UBlackboardComponent* BlackboardComp = AIController->GetBlackboardComponent())
		{
			BlackboardComp->SetValueAsObject("TargetActor", nullptr);
		}

		AIController->StopMovement();//이동 중지
		AIController->SetStop();
	}

	GetCharacterMovement()->StopMovementImmediately();//이동(관성) 즉시 중지, 물리적인거라 StopMovement랑 다르다고 함
	GetCharacterMovement()->DisableMovement();//이동 비활성
}

void ABaseMonsterCharacter::DestroyActor()
{
	if (HasAuthority() && IsValid(this))
	{
		Destroy();
	}
}

void ABaseMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABaseMonsterCharacter, CurrentHP);
	DOREPLIFETIME(ABaseMonsterCharacter, bIsDead);
	DOREPLIFETIME(ABaseMonsterCharacter, bIsAttacking);
}


void ABaseMonsterCharacter::PerformAttack()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown || bIsAttacking)
		return;

	if (GetLocalRole() == ROLE_Authority)
	{
		ServerPerformAttack();
	}
}

void ABaseMonsterCharacter::ServerPerformAttack_Implementation()
{
	float CurrentTime = GetWorld()->GetTimeSeconds();
	if (CurrentTime - LastAttackTime < AttackCooldown || bIsAttacking)
		return;

	LastAttackTime = CurrentTime;
	bIsAttacking = true;

	MulticastStartAttack();
	GetWorldTimerManager().SetTimer(AttackTimerHandle, this, &ABaseMonsterCharacter::OnAttackFinished, AttackDuration, false);
}

void ABaseMonsterCharacter::OnAttackFinished()
{
	bIsAttacking = false;
	DisableAttackCollider();
}

void ABaseMonsterCharacter::MulticastStartAttack_Implementation()
{
	if (!IsValid(this) || bIsDead) return;

	if (IsValid(StartAttack))
	{
		PlayAnimMontage(StartAttack);

		GetWorldTimerManager().SetTimer(AttackEnableTimerHandle, this,
			&ABaseMonsterCharacter::EnableAttackCollider, 0.5f, false);

		int32 RandomSound = FMath::RandRange(0, 2);
		switch (RandomSound)
		{
		case 0:
			PlayAttackSound1();
			break;
		case 1:
			PlayAttackSound2();
			break;
		case 2:
			PlayAttackSound3();
			break;
		}
	}
}

void ABaseMonsterCharacter::MulticastAIMove_Implementation()
{
	if (IsValid(AImove))
	{
		PlayAnimMontage(AImove);
	}
}

void ABaseMonsterCharacter::MulticastAIDeath_Implementation()
{
	if (IsValid(AIDeath))
	{
		PlayAnimMontage(AIDeath);

		(FMath::RandBool()) ? PlayDeathSound1() : PlayDeathSound2();
	}
}

void ABaseMonsterCharacter::EnableAttackCollider()
{
	if (!bIsDead)
	{
		AttackCollider->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	}
}

void ABaseMonsterCharacter::DisableAttackCollider()
{
	AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ABaseMonsterCharacter::GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const
{
	TagContainer = GameplayTags;
}

void ABaseMonsterCharacter::MulticastPlaySound_Implementation(USoundBase* Sound)
{
	if (Sound)
	{
		UGameplayStatics::PlaySoundAtLocation(this,
			Sound,
			GetActorLocation(),
			FRotator::ZeroRotator,
			2.0f);
	}
}

void ABaseMonsterCharacter::PlayAttackSound1()
{
	if (AttackSound1)
	{
		MulticastPlaySound(AttackSound1);
	}
}

void ABaseMonsterCharacter::PlayAttackSound2()
{
	if (AttackSound2)
	{
		MulticastPlaySound(AttackSound2);
	}
}

void ABaseMonsterCharacter::PlayAttackSound3()
{
	if (AttackSound3)
	{
		MulticastPlaySound(AttackSound3);
	}
}

void ABaseMonsterCharacter::PlayDeathSound1()
{
	if (DeathSound1)
	{
		MulticastPlaySound(DeathSound1);
	}
}

void ABaseMonsterCharacter::PlayDeathSound2()
{
	if (DeathSound2)
	{
		MulticastPlaySound(DeathSound2);
	}
}

void ABaseMonsterCharacter::PlayIdleSound()
{
	if (IdleSound)
	{
		MulticastPlaySound(IdleSound);
	}
}

void ABaseMonsterCharacter::PlayMoveSound()
{
	if (MoveSound)
	{
		MulticastPlaySound(MoveSound);
	}
}

void ABaseMonsterCharacter::PlayChaseSound()
{
	int32 RandomSound = FMath::RandRange(0, 2);
	switch (RandomSound)
	{
	case 0:
		PlayChaseSound1();
		break;
	case 1:
		PlayChaseSound2();
		break;
	case 2:
		PlayChaseSound3();
		break;
	}
}

void ABaseMonsterCharacter::PlayChaseSound1()
{
	if (ChaseSound1)
	{
		MulticastPlaySound(ChaseSound1);
	}
}

void ABaseMonsterCharacter::PlayChaseSound2()
{
	if (ChaseSound2)
	{
		MulticastPlaySound(ChaseSound2);
	}
}

void ABaseMonsterCharacter::PlayChaseSound3()
{
	if (ChaseSound3)
	{
		MulticastPlaySound(ChaseSound3);
	}
}

void ABaseMonsterCharacter::EnableStencilForAllMeshes(int32 StencilValue)
{
	TArray<UMeshComponent*> MeshComponents;
	GetComponents<UMeshComponent>(MeshComponents);

	for (UMeshComponent* MeshComp : MeshComponents)
	{
		MeshComp->SetRenderCustomDepth(true);
		MeshComp->SetCustomDepthStencilValue(StencilValue);
	}
}