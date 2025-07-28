#include "Character/Component/CharacterInteractionComponent.h"
#include "Character/BaseCharacter.h"
#include "LastCanary.h"

void UCharacterInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    SetComponentTickEnabled(true);

    if (GetCharacter())
    {
        CachedController = Cast<APlayerController>(GetCharacter()->GetController());
    }
}

void UCharacterInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    PerformTrace();
}

void UCharacterInteractionComponent::PerformTrace()
{
    
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!GetCharacter() || !GetCharacter()->IsLocallyControlled())
    {
        UpdateFocus(nullptr);
        return;
    }

    // 카메라 위치와 방향 얻기
    FVector Start;
    FRotator ViewRot;

    if (!GetPlayerController())
    {
        UpdateFocus(nullptr);
        return;
    }

    GetPlayerController()->GetPlayerViewPoint(Start, ViewRot);

    FVector End = Start + (ViewRot.Vector() * TraceDistance);

    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(OwnerPawn);

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

    if (bHit)
    {
        UpdateFocus(Hit.GetActor());
    }
    else
    {
        UpdateFocus(nullptr);
    }

#if WITH_EDITOR
    DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 0.1f);
#endif
}

void UCharacterInteractionComponent::UpdateFocus(AActor* NewActor)
{
    if (CurrentFocusedActor != NewActor)
    {
        CurrentFocusedActor = NewActor;
        OnFocusChanged.Broadcast(NewActor);
    }
}