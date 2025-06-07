#include "AI/BaseMonsterCharacter.h"
#include "AI/BaseAIController.h"
#include "Character/BaseCharacter.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Touch.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "GameFramework/Character.h"
#include "Engine/DamageEvents.h" 
#include "Net/UnrealNetwork.h"

ABaseMonsterCharacter::ABaseMonsterCharacter()
{
    PrimaryActorTick.bCanEverTick = false;
    bReplicates = true;

    MaxHP = 100;
    CurrentHP = MaxHP;
    bIsDead = false;

    AttackDamage = 10.0f;

    AIPerceptionComponent = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("PerceptionComponent"));

    AIControllerClass = ABaseAIController::StaticClass();
    AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

    AttackCollider = CreateDefaultSubobject<USphereComponent>(TEXT("AttackCollider"));
    AttackCollider->SetupAttachment(RootComponent);//GetMesh(), FName("hand_r"));
    //AttackCollider->SetRelativeLocation(FVector(100, 0, 0));
    AttackCollider->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    //공격 이벤트 바인딩
    AttackCollider->OnComponentBeginOverlap.AddDynamic(this, &ABaseMonsterCharacter::OnAttackHit);

    UCapsuleComponent* CapsuleComp = GetCapsuleComponent();
    if (CapsuleComp)
    {
        CapsuleComp->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        CapsuleComp->SetCollisionResponseToChannel(ECC_Pawn, ECR_Block);
    }

    SetReplicateMovement(true);

    GetCharacterMovement()->bOrientRotationToMovement = true;
    GetCharacterMovement()->MaxWalkSpeed = 200.f;

    NavGenerationradius = 200.0f;
    NavRemovalradius = 300.0f;

    NavInvoker = CreateDefaultSubobject<UNavigationInvokerComponent>(TEXT("NavInvoker"));
    NavInvoker->SetGenerationRadii(NavGenerationradius, NavRemovalradius);

    GameplayTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Character.Enemy")));
    if (GetMesh())
    {
        GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
    }
}

float ABaseMonsterCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
    class AController* EventInstigator, AActor* DamageCauser)
{
    if (!HasAuthority()) return 0.0f;
    if (!IsValid(DamageCauser)) return 0.0f;
    if (bIsDead) return 0.0f;

    float DamageApplied = FMath::Clamp(DamageAmount, 0.0f, CurrentHP);
    CurrentHP -= DamageAmount;

    if (CurrentHP <= 0)
    {
        CurrentHP = 0;
        bIsDead = true;

        GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
        GetCharacterMovement()->SetMovementMode(MOVE_None);

        MulticastAIDeath();

        if (HasAuthority())
        {
            GetWorldTimerManager().SetTimer(DeathTimerHandle, this,
                &ABaseMonsterCharacter::DestroyActor, 1.9f, false);
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

    // 캐릭터인지 확인
    if (ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(OtherActor))
    {
        //데미지
        FDamageEvent DamageEvent;
        HitCharacter->TakeDamage(AttackDamage, DamageEvent, GetController(), this);

        //콜라이더 비활성화
        DisableAttackCollider();
    }
}

void ABaseMonsterCharacter::DestroyActor()
{
    if (HasAuthority())
    {
        //UE_LOG(LogTemp, Warning, TEXT("Death"));
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
    DisableAttackCollider(); // 공격 끝나면 콜라이더 비활성화
}

void ABaseMonsterCharacter::MulticastStartAttack_Implementation()
{
    if (IsValid(StartAttack))
    {
        PlayAnimMontage(StartAttack);

        // 공격 시작 후 약간의 딜레이 후 콜라이더 활성화
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