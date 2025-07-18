#include "Character/Component/CharacterAnimationComponent.h"
#include "Character/BaseCharacter.h"

#include "LastCanary.h"

UCharacterAnimationComponent::UCharacterAnimationComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterAnimationComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsValid(CachedCharacter))
	{
		USkeletalMeshComponent* Mesh = CachedCharacter->GetMesh();
		if (IsValid(Mesh))
		{
			CachedAnimInstance = Mesh->GetAnimInstance();
		}
	}
}

void UCharacterAnimationComponent::PlayMontageByType(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype)
{

}

void UCharacterAnimationComponent::PlayInteractMontage()
{

}

void UCharacterAnimationComponent::PlayUseItemMontage()
{

}

void UCharacterAnimationComponent::PlayGunReloadMontage()
{

}

void UCharacterAnimationComponent::PlayEmoteMontage()
{

}

void UCharacterAnimationComponent::PlayAttackMontage()
{

}


void UCharacterAnimationComponent::Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype)
{

}

void UCharacterAnimationComponent::Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype)
{

}

void UCharacterAnimationComponent::CancelMontageByType(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype)
{
}

void UCharacterAnimationComponent::CancelInteractModntage()
{
}

void UCharacterAnimationComponent::CancelUseItemMontage()
{
}

void UCharacterAnimationComponent::CancelGunReloadMontage()
{
}

void UCharacterAnimationComponent::CancelEmoteMontage()
{
}

void UCharacterAnimationComponent::CancelAttackMontage()
{
}

void UCharacterAnimationComponent::Server_CancelMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype)
{
}

void UCharacterAnimationComponent::Multicast_CancelMontage_Implementation(UAnimMontage* MontageToPlay, EAnimationMontageType Animtype)
{
}
