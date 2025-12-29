#include "Character/Component/CharacterInteractionComponent.h"
#include "Character/BaseCharacter.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "Item/ItemBase.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Actor/Gimmick/LCBaseGimmick.h"

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
    if (!IsValid(GetBaseCharacter()) || !GetBaseCharacter()->IsLocallyControlled() || !IsValid(GetPlayerController()))
    {
        UpdateFocus(nullptr);
        return;
    }

    // 카메라 위치
    FVector ViewLocation;
    FRotator ViewRotation;
    
    GetPlayerController()->GetPlayerViewPoint(ViewLocation, ViewRotation);

    // 거리 계산 (FPS/3인칭에 따른 변경은 캐릭터에서 설정 가능하게)
    float Distance = GetBaseCharacter()->bIsFPSCamera? TraceDistance : TraceDistance * 3.f;
    FVector End = ViewLocation + ViewRotation.Vector() * Distance;

    // 라인트레이스
    FHitResult Hit;
    FCollisionQueryParams Params;
    Params.AddIgnoredActor(GetOwner());

    bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, ViewLocation, End, ECC_Visibility, Params);

    // 벽 가까움 체크
    GetBaseCharacter()->bIsCloseToWall = bHit && Hit.Distance < 100.f;

    UpdateFocus(bHit ? Hit.GetActor() : nullptr);
    //DrawDebugLine(GetWorld(), ViewLocation, End, FColor::Green, false, 0.1f);
}

void UCharacterInteractionComponent::UpdateFocus(AActor* NewActor)
{
    if (CurrentFocusedActor != NewActor)
    {
        CurrentFocusedActor = NewActor;
        OnFocusChanged.Broadcast(NewActor);
    }
}

void UCharacterInteractionComponent::Handle_Interact()
{
    if (!IsValid(CurrentFocusedActor))
    {
        return;
    }

    if (!CanInteract())
    {
        return;
    }

    SetRecentInteractedActor(CurrentFocusedActor);

    if (CheckInteractDirectly())
    {
        if (IsValid(GetRecentInteractedActor()))
        {
            Interact(GetRecentInteractedActor());
        }
        return;
    }
    GetBaseCharacter()->AnimationComponent->PlayInteractMontage(GetRecentInteractedActor());
}

bool UCharacterInteractionComponent::CanInteract()
{
	if (!IsValid(CurrentFocusedActor))
	{
		return false;
	}
	if (CurrentFocusedActor->IsA<AItemBase>())
	{
		AItemBase* Item = Cast<AItemBase>(CurrentFocusedActor);

		if (!IsValid(Item))
		{
			return false;
		}

		if (!GetBaseCharacter()->ToolbarInventoryComponent->CanAddItem(Item))
		{
			if (!Item->IsCollectible())
			{
				return false;
			}

			if (!GetBaseCharacter()->bBackpackMeshActive)
			{
				return false;
			}
		}
	}

    return true;
}

bool UCharacterInteractionComponent::CheckInteractDirectly()
{
    if (GetRecentInteractedActor()->IsA<ALCBaseGimmick>())
    {
        ALCBaseGimmick* Gimmick = Cast<ALCBaseGimmick>(GetRecentInteractedActor());

        if (!IsValid(Gimmick))
        {
            return false;
        }

        if (!Gimmick->bPlayCharacterAnimation) // 기믹인데, 애니메이션 재생이 필요 없다고 판정되면
        {
            return true;
        }
    }
    else if (GetRecentInteractedActor()->IsA<AItemBase>()) //아이템이면 반드시 줍는 상태일 것 -> 반드시 줍는 애니메이션 재생
    {
        return false;
    }

    //나머지는 PC에 인터랙트, 혹은 게이트일 가능성
    return true;
}

void UCharacterInteractionComponent::Interact(AActor* Actor)
{
    IInteractableInterface::Execute_Interact(Actor, GetPlayerController());
}

void UCharacterInteractionComponent::Interact()
{
    IInteractableInterface::Execute_Interact(GetRecentInteractedActor(), GetPlayerController());
}