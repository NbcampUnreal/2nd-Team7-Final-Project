// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacterAttackNotify.h"
#include "GameFramework/Actor.h"
#include "Character/BaseCharacter.h"
#include "Kismet/GameplayStatics.h" // 꼭 포함 필요

void UBaseCharacterAttackNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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

    if (AttackTimingType == ECharacterAttackTimingType::Trigger)
    {
        StartAttack(MeshComp, Animation);
    }
    else if (AttackTimingType == ECharacterAttackTimingType::End)
    {
        EndAttack(MeshComp, Animation);
    }
}

void UBaseCharacterAttackNotify::StartAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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

    switch (NotifyType)
    {
    case ECharacterAttackType::Punch:
        break;
    case ECharacterAttackType::Kick:
        if (Player->HasAuthority())
        {
            Player->StartKickHit();
        }
        break;
    default:
        break;
    }
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(Player, SoundToPlay, Player->GetActorLocation());
    }
}

void UBaseCharacterAttackNotify::EndAttack(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
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

    switch (NotifyType)
    {
    case ECharacterAttackType::Punch:
        break;
    case ECharacterAttackType::Kick:
        if (Player->HasAuthority())
        {
            Player->EndKickHit();
        }
        break;
    default:
        break;
    }
    if (SoundToPlay)
    {
        UGameplayStatics::PlaySoundAtLocation(Player, SoundToPlay, Player->GetActorLocation());
    }
}