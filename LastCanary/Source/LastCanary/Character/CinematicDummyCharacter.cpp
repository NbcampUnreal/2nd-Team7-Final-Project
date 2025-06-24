#include "Character/CinematicDummyCharacter.h"
#include "TimerManager.h"
#include "GameFramework/CharacterMovementComponent.h"

ACinematicDummyCharacter::ACinematicDummyCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsMoving = false;

    // 더미는 AI도 입력도 안 받음
    AutoPossessAI = EAutoPossessAI::Disabled;
    AutoPossessPlayer = EAutoReceiveInput::Disabled;

    GetCharacterMovement()->bOrientRotationToMovement = true;
    bUseControllerRotationYaw = false;
}

void ACinematicDummyCharacter::BeginPlay()
{
    Super::BeginPlay();
}

void ACinematicDummyCharacter::ApplyAppearance(const FCharacterAppearanceData& Data)
{
    if (Data.BodyMesh)
    {
        GetMesh()->SetSkeletalMesh(Data.BodyMesh);
    }
    if (Data.AnimBP)
    {
        GetMesh()->SetAnimInstanceClass(Data.AnimBP);
    }
    for (int32 i = 0; i < Data.Materials.Num(); ++i)
    {
        GetMesh()->SetMaterial(i, Data.Materials[i]);
    }
}

void ACinematicDummyCharacter::MoveToLocation(const FVector& TargetLocation, float Duration)
{
    if (Duration <= 0.f)
    {
        return;
    }

    MoveStart = GetActorLocation();
    MoveEnd = TargetLocation;
    MoveDuration = Duration;
    MoveElapsed = 0.f;
    bIsMoving = true;
}

void ACinematicDummyCharacter::PlayAnimMontageOnce(UAnimMontage* Montage)
{
    if (Montage)
    {
        PlayAnimMontage(Montage);
    }
}

void ACinematicDummyCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (bIsMoving)
    {
        TickMove(DeltaSeconds);
    }
}

void ACinematicDummyCharacter::TickMove(float DeltaTime)
{
    MoveElapsed += DeltaTime;
    float Alpha = FMath::Clamp(MoveElapsed / MoveDuration, 0.f, 1.f);
    FVector NewLocation = FMath::Lerp(MoveStart, MoveEnd, Alpha);
    SetActorLocation(NewLocation);

    if (Alpha >= 1.f)
    {
        bIsMoving = false;
    }
}
