#include "LC_CameraManager.h"
#include "Character/BaseCharacter.h"

void ALC_CameraManager::UpdateCamera(float DeltaTime)
{
	Super::UpdateCamera(DeltaTime);

	AActor* ViewTargetActor = GetViewTarget();
	ABaseCharacter* Character = Cast<ABaseCharacter>(ViewTargetActor);
	if (!Character)
	{
		return;
	}

	if (!Character->IsADS())
	{
		return;
	}

	const FTransform AdsTransform = Character->GetADSCameraTransform();

	ViewTarget.POV.Location = AdsTransform.GetLocation();
	ViewTarget.POV.Rotation = AdsTransform.Rotator();
}


void ALC_CameraManager::UpdateViewTargetInternal(
	FTViewTarget& OutVT,
	float DeltaTime
)
{
	Super::UpdateViewTargetInternal(OutVT, DeltaTime);

	ABaseCharacter* Char =
		Cast<ABaseCharacter>(OutVT.Target);

	if (!Char)
	{
		return;
	}

	// 캐릭터가 제공하는 "의도"
	const FVector CameraOffset = Char->GetDesiredCameraOffset();

	// 카메라 회전 기준으로 로컬 오프셋 적용
	OutVT.POV.Location = Char->GetPawnViewLocation();
	OutVT.POV.Location +=
		OutVT.POV.Rotation.RotateVector(
			Char->GetDesiredCameraOffset()
		);


}
