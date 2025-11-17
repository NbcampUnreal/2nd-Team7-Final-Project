#include "LCRotationHandle.h"
#include "Interface/LCGimmickInterface.h"
#include "Components/StaticMeshComponent.h"
#include "LastCanary.h"

ALCRotationHandle::ALCRotationHandle()
	: ALCRotationGimmick()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	RotationStep = 45.f;
	RotationSpeed = 45.f;
	bUseAxis = false;
}

#pragma region Rotation

void ALCRotationHandle::StartRotation()
{
	Super::StartRotation();

	Multicast_PlaySound();

	for (AActor* Target : LinkedTargets)
	{
		if (!IsValid(Target))
			continue;

		if (Target->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
		{
			if (ILCGimmickInterface::Execute_CanActivate(Target))
			{
				ILCGimmickInterface::Execute_ActivateGimmick(Target);
			}
		}
	}
}

#pragma endregion
