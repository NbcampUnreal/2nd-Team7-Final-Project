#include "Character/AnimNotify/LCActionNotify.h"
#include "Character/BaseCharacter.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "Item/ItemBase.h"

void ULCActionNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!IsValid(MeshComp)) return;

	ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(Character)) return;

	UCharacterAnimationComponent* AnimComp = Character->AnimationComponent;
	if (!IsValid(AnimComp)) return;

	// 실제 아이템 사용 로직 트리거
	AnimComp->HandleAnimNotify(EAnimationMontageType::UseItem); // 아래에 구현할 함수
}