#include "RuinsRotatingStatue.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ARuinsRotatingStatue::ARuinsRotatingStatue()
	: RotationStep(45.f)
	, RotationIndex(0)   
	, RotateSpeed(60.f)
	, bIsRotating(false)
	, TargetYaw(0.f)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true); 
	bAlwaysRelevant = true;
}

void ARuinsRotatingStatue::ActivateGimmick_Implementation()
{
	Super::ActivateGimmick_Implementation();

	if (bIsRotating)
	{
		LOG_Art_WARNING(TEXT("동상이 회전 중입니다. 중복 회전 요청 무시"));
		return;
	}

	StartRotation();
}

void ARuinsRotatingStatue::StartRotation()
{
	bIsRotating = true;

	RotationIndex = (RotationIndex + 1) % FMath::RoundToInt(360.f / RotationStep);

	TargetYaw = RotationIndex * RotationStep;

	LOG_Art(Log, TEXT("▶ 회전 시작 - TargetYaw: %.1f"), TargetYaw);
}

// 회전 완료 처리
void ARuinsRotatingStatue::FinishRotation()
{
	bIsRotating = false;

	LOG_Art(Log, TEXT("▶ 회전 완료 - 최종 Yaw: %.1f"), GetActorRotation().Yaw);
}

void ARuinsRotatingStatue::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!HasAuthority() || !bIsRotating)
	{
		return;
	}

	FRotator CurrentRotation = GetActorRotation();
	float NewYaw = FMath::FInterpConstantTo(CurrentRotation.Yaw, TargetYaw, DeltaSeconds, RotateSpeed);
	SetActorRotation(FRotator(CurrentRotation.Pitch, NewYaw, CurrentRotation.Roll));

	if (FMath::IsNearlyEqual(NewYaw, TargetYaw, 0.1f))
	{
		SetActorRotation(FRotator(CurrentRotation.Pitch, TargetYaw, CurrentRotation.Roll));
		FinishRotation();
	}
}