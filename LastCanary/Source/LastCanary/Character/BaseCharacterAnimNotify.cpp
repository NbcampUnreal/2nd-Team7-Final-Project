// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BaseCharacterAnimNotify.h"
#include "GameFramework/Actor.h"
#include "Character/BaseCharacter.h"

void UBaseCharacterAnimNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    if (AActor* Owner = MeshComp->GetOwner())
    {
        // 캐릭터로 캐스팅
        if (ABaseCharacter* Player = Cast<ABaseCharacter>(Owner))
        {
            // 캐릭터가 가지고 있는 TargetActor 사용해서 함수 호출
            if (Player->InteractTargetActor)
            {
                Player->OnNotified();
            }
            else
            {
                UE_LOG(LogTemp, Warning, TEXT("TargetActor is null!"));
            }
        }
    }
}