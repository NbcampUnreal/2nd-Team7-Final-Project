#include "Character/Component/CharacterInteractionComponent.h"
#include "Character/BaseCharacter.h"
#include "LastCanary.h"

void UCharacterInteractionComponent::BeginPlay()
{
    Super::BeginPlay();

    SetComponentTickEnabled(true);
}

void UCharacterInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    PerformTrace();
}

void UCharacterInteractionComponent::PerformTrace()
{
    if (!GetCharacter() || !GetCharacter()->IsLocallyControlled())
    {
        UpdateFocus(nullptr);
        return;
    }

    // 카메라 위치
    FVector ViewLocation;
    FRotator ViewRotation;
    if (!GetPlayerController())
    {
        UpdateFocus(nullptr);
        return;
    }
    GetPlayerController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

    // 거리 계산 (FPS/3인칭에 따른 변경은 캐릭터에서 설정 가능)
    float Distance = GetCharacter()->bIsFPSCamera? TraceDistance : TraceDistance * 3.f;
    FVector End = ViewLocation + ViewRotation.Vector() * Distance;

    // 라인트레이스
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Visibility, Params);

    // 벽 가까움 체크
    GetCharacter()->bIsCloseToWall = bHit && Hit.Distance < 100.f;

    UpdateFocus(bHit ? Hit.GetActor() : nullptr);
    DrawDebugLine(GetWorld(), ViewLocation, End, FColor::Green, false, 0.1f);
}

void UCharacterInteractionComponent::UpdateFocus(AActor* NewActor)
{
    if (CurrentFocusedActor != NewActor)
    {
        CurrentFocusedActor = NewActor;
        OnFocusChanged.Broadcast(NewActor);
    }
}