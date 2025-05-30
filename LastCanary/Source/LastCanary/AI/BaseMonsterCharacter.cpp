#include "AI/BaseMonsterCharacter.h"
#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Touch.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

ABaseMonsterCharacter::ABaseMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    MaxHP = 100;
    CurrentHP = MaxHP;
    bIsDead = false;

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    AIControllerClass = ABaseAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("AttackCollider"));
    AttackCollider->SetupAttachment(GetMesh(), FName("hand_r"));
    AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    //AttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABaseMonsterCharacter::OnAttackHit);

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        //CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }

    SetReplicateMovement(true);

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;

    NavGenerationradius = 200.0f;
    NavRemovalradius = 300.0f;

    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
    NavInvoker->SetGenerationRadii(NavGenerationradius, NavRemovalradius);
}

//void ABaseMonsterCharacter::OnAttackHit(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
//{
//    UE_LOG(LogTemp, Warning, TEXT("Damage: %s"), *OtherActor->GetName());
//}

float ABaseMonsterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.0f;
    if (!IsValid(DamageCauser)) return 0.0f;

    float DamageApplied = FMath::Clamp(DamageAmount, 0.0f, CurrentHP);
    CurrentHP -= DamageAmount;

    //TO DO : Add Hit Motion

    if (CurrentHP <= 0)
    {
        CurrentHP = 0;
        bIsDead = true;
        MulticastAIDeath();

        // ���������� ���� �� Destroy ó��
        if (HasAuthority())
        {
            GetWorldTimerManager().SetTimer(DeathTimerHandle, this,
                &ABaseMonsterCharacter::DestroyActor, 2.0f, false);
        }
    }

    return DamageApplied;
}

void ABaseMonsterCharacter::DestroyActor()
{
    if (HasAuthority())
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

void ABaseMonsterCharacter::BeginPlay()
{
    Super::BeginPlay();
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
}

void ABaseMonsterCharacter::MulticastStartAttack_Implementation()
{
    //UE_LOG(LogTemp, Warning, TEXT("Attack"));
    if (IsValid(StartAttack))
    {
        PlayAnimMontage(StartAttack);

    }
}

void ABaseMonsterCharacter::MulticastAIMove_Implementation()
{
    //UE_LOG(LogTemp, Warning, TEXT("Move"));
    if (IsValid(AImove))
    {
        PlayAnimMontage(AImove);
    }
}

void ABaseMonsterCharacter::MulticastAIDeath_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Death"));
    if (IsValid(AIDeath))
    {
        PlayAnimMontage(AIDeath);
        // Destroy() ����! ���������� ó����
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