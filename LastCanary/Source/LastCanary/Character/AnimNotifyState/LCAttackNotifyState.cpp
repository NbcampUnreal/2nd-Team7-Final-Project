#include "Character/AnimNotifyState/LCAttackNotifyState.h"
#include "Character/Component/CharacterAttackComponent.h"

void ULCAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* MeleeComp = Owner->FindComponentByClass<UCharacterAttackComponent>())
		{
			MeleeComp->StartAttack();
		}
	}
}

void ULCAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (AActor* Owner = MeshComp->GetOwner())
	{
		if (auto* MeleeComp = Owner->FindComponentByClass<UCharacterAttackComponent>())
		{
			MeleeComp->EndAttack();
		}
	}
}