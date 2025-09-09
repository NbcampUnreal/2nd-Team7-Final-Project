#include "Character/Component/CharacterFootstepNoiseComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"

UCharacterFootstepNoiseComponent::UCharacterFootstepNoiseComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCharacterFootstepNoiseComponent::BeginPlay()
{
    Super::BeginPlay();
    UpdateFootstepState();
}

void UCharacterFootstepNoiseComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    UpdateFootstepState();
}

void UCharacterFootstepNoiseComponent::UpdateFootstepState()
{
    ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter) return;

    UCharacterMovementComponent* MoveComp = OwnerCharacter->GetCharacterMovement();
    if (!MoveComp) return;

    if (MoveComp->IsFalling())
    {
        CurrentFootstepState = EFootstepState::None;
        NoiseLevel = 0.f;
        return;
    }

    if (MoveComp->IsCrouching())
    {
        CurrentFootstepState = EFootstepState::Crouch;
        NoiseLevel = CrouchNoise;
    }
    else if (MoveComp->Velocity.Size() > MoveComp->MaxWalkSpeed * 0.75f)
    {
        CurrentFootstepState = EFootstepState::Run;
        NoiseLevel = RunNoise;
    }
    else if (MoveComp->Velocity.Size() > 10.f)
    {
        CurrentFootstepState = EFootstepState::Walk;
        NoiseLevel = WalkNoise;
    }
    else
    {
        CurrentFootstepState = EFootstepState::None;
        NoiseLevel = 0.f;
    }
}

float UCharacterFootstepNoiseComponent::GetCurrentFootstepNoiseLevel() const
{
    return NoiseLevel;
}

EFootstepState UCharacterFootstepNoiseComponent::GetCurrentFootstepState() const
{
    return CurrentFootstepState;
}