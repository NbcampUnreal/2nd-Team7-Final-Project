#include "Character/AnimNotify/LCInteractionNotify.h"
#include "Character/BaseCharacter.h"
#include "Character/Component/CharacterAnimationComponent.h"
#include "Item/ItemBase.h"

void ULCInteractionNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	if (!IsValid(MeshComp)) return;

	ABaseCharacter* Character = Cast<ABaseCharacter>(MeshComp->GetOwner());
	if (!IsValid(Character)) return;

	UCharacterAnimationComponent* AnimComp = Character->AnimationComponent;
	if (!IsValid(AnimComp)) return;

		
	if (!Character->HasAuthority()) return;	// 서버에서만 실행하기
	
	// 인터랙션 트리거
	AnimComp->HandleAnimNotify(EAnimationMontageType::Interaction); // 아래에 구현할 함수
}