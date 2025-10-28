#include "Character/AnimNotifyState/LCAttackNotifyState.h"
#include "Character/Component/CharacterAttackComponent.h"

void ULCAttackNotifyState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{	
	HandleStartAttackByType(MeshComp, Animation, TotalDuration);
}

void ULCAttackNotifyState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	HandleEndAttackByType(MeshComp, Animation);
}

void ULCAttackNotifyState::HandleStartAttackByType(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
	//Attack의 경우는 무조건 서버에서만 돌리기

	UCharacterAttackComponent* AttackComponent;
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}
	
	AttackComponent = Owner->FindComponentByClass<UCharacterAttackComponent>();
	if (!IsValid(AttackComponent))
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		switch (AttackType)
		{
		case EAttackType::Punch:
			break;
		case EAttackType::Kick:
			AttackComponent->EnableKickHitBox();
			break;
		case EAttackType::ItemAttack:
			AttackComponent->StartItemAttack();
			break;
		default:
			break;
		}
		return;
	}
	else
	{

	}

}

void ULCAttackNotifyState::HandleEndAttackByType(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	//Attack의 경우는 무조건 서버에서만 돌리기

	UCharacterAttackComponent* AttackComponent;
	AActor* Owner = MeshComp->GetOwner();
	if (!IsValid(Owner))
	{
		return;
	}
	
	AttackComponent = Owner->FindComponentByClass<UCharacterAttackComponent>();
	if (!IsValid(AttackComponent))
	{
		return;
	}

	if (Owner->HasAuthority())
	{
		switch (AttackType)
		{
		case EAttackType::Punch:
			break;
		case EAttackType::Kick:
			AttackComponent->DisableKickHitBox();
			break;
		case EAttackType::ItemAttack:
			AttackComponent->EndItemAttack();
			break;
		default:
			break;
		}
		return;
	}
	else
	{
		switch (AttackType)
		{
		case EAttackType::Punch:
			break;
		case EAttackType::Kick:
			AttackComponent->DisableKickHitBox();
			break;
		case EAttackType::ItemAttack:
			AttackComponent->EndItemAttack();
			break;
		default:
			break;
		}
		return;
	}
	
}
