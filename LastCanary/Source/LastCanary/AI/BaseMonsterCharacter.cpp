#include "AI/BaseMonsterCharacter.h"
#include "AI/BaseAIController.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Touch.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

ABaseMonsterCharacter::ABaseMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    MaxHP = 100;
    CurrentHP = MaxHP;
    bIsDead = false;

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));
    //SensoryType = ECC_Visibility;

    AIControllerClass = ABaseAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    SetReplicateMovement(true);

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 300.f;

    NavGenerationradius = 10.0f;
    NavRemovalradius = 15.0f;

    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
    NavInvoker->SetGenerationRadii(NavGenerationradius, NavRemovalradius);
}

float ABaseMonsterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.0f;
    if (!IsValid(DamageCauser)) return 0.0f;

    float DamageApplied = FMath::Clamp(DamageAmount, 0.0f, CurrentHP);
    CurrentHP -= DamageAmount;

    if (CurrentHP <= 0)
    {
        CurrentHP = 0;
        bIsDead = true;
        MulticastAIDeath();

        //SetLifeSpan(5.0f);
    }

    return DamageApplied;
}

void ABaseMonsterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    // 중요 변수 복제 설정
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
    UE_LOG(LogTemp, Warning, TEXT("Attack"));
    if (IsValid(StartAttack))
    {
        PlayAnimMontage(StartAttack);
        
    }
}

void ABaseMonsterCharacter::MulticastAIMove_Implementation()
{
    UE_LOG(LogTemp, Warning, TEXT("Move"));
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
        
    }
}