// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacterAnimNotify.h"
#include "GameFramework/Actor.h"
#include "Character/BaseCharacter.h"

void UBaseCharacterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    AActor* Owner = MeshComp->GetOwner();
    if (!IsValid(Owner))
    {
        return;
    }
    ABaseCharacter* Player = Cast<ABaseCharacter>(Owner);
    // 캐릭터로 캐스팅
    if (!IsValid(Player))
    {
        return;
    }
    if (!Player->IsLocallyControlled())
    {
        return;
    }
    switch (NotifyType)
    {
    case ECharacterNotifyType::Interact:
        if (Player->InteractTargetActor)
        {
            if (Player->IsLocallyControlled())
            {
                Player->OnNotified();
            }
        }
        break;
    case ECharacterNotifyType::ItemUse:
        Player->UseItemAnimationNotified();
        break;
    case ECharacterNotifyType::Reload:
        Player->GunReloadAnimationNotified();
        break;
    default:
        break;
    }
}