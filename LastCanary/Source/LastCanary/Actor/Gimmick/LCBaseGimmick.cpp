#include "Actor/Gimmick/LCBaseGimmick.h"
#include "Interface/GimmickEffectInterface.h"

ALCBaseGimmick::ALCBaseGimmick()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
}

void ALCBaseGimmick::ActivateGimmick_Implementation()
{
	if (HasAuthority() == false)
	{
		Server_ActivateGimmick();
		return;
	}

	if (bActivated == true)
	{
		return;
	}

	bActivated = true;

	for (AActor* Target : LinkedTargets)
	{
		if (!IsValid(Target))
		{
			continue;
		}

		if (Target->GetClass()->ImplementsInterface(UGimmickEffectInterface::StaticClass()))
		{
			IGimmickEffectInterface::Execute_TriggerEffect(Target);
		}
	}
}

void ALCBaseGimmick::Server_ActivateGimmick_Implementation()
{
	ActivateGimmick_Implementation(); // 서버에서 재실행
}

bool ALCBaseGimmick::CanActivate_Implementation()
{
	return true; // 자식에서 override
}

void ALCBaseGimmick::DeactivateGimmick_Implementation()
{
	// 기본은 아무 것도 안 함. 자식 클래스에서 오버라이드
}
