#include "Actor/Gimmick/LCAutoGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCAutoGimmick::ALCAutoGimmick()
	: MovePauseTime(0.5f)
	, ReturnPauseTime(0.5f)
	, bLoopingEnabled(true)
{
	ActivationType = EGimmickActivationType::ActivateOnConditionMet;
	bToggleState = false;
}

void ALCAutoGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		StartLoop();
	}
}

void ALCAutoGimmick::StartLoop()
{
	AutoMove();
}

void ALCAutoGimmick::AutoMove()
{
	if (!bLoopingEnabled) return;
	if (!HasAuthority() || bIsMovingServer || bIsReturningServer || bIsRotatingServer || bIsReturningRotationServer)
		return;

	InitialLocation = GetActorLocation();
	TargetLocation = InitialLocation + MoveVector;

	StartServerMovement(InitialLocation, TargetLocation, MoveDuration);
	Multicast_StartMovement(InitialLocation, TargetLocation, MoveDuration);

	FVector AxisVector = FVector::ForwardVector;
	switch (RotationAxisEnum)
	{
	case EGimmickRotationAxis::X: AxisVector = FVector::RightVector; break;
	case EGimmickRotationAxis::Y: AxisVector = FVector::UpVector;    break;
	case EGimmickRotationAxis::Z: default:                           break;
	}

	TotalRotationIndex += RotationCount;
	const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * TotalRotationIndex);
	const FQuat DeltaQuat = FQuat(AxisVector, TotalAngleRad);
	const FQuat ResultQuat = OriginalRotationQuat * DeltaQuat;

	StartServerRotation(CurrentRotationQuat, ResultQuat, RotationDuration);
	Multicast_StartRotation(CurrentRotationQuat, ResultQuat, RotationDuration);

	CurrentRotationQuat = ResultQuat;

	FTimerDelegate Delegate;
	Delegate.BindLambda([this]()
		{
			SetActorLocation(TargetLocation);
			bIsMovingServer = false;
			bIsRotatingServer = false;

			GetWorldTimerManager().SetTimer(
				AutoReturnTimerHandle,
				this,
				&ALCAutoGimmick::AutoReturn,
				MovePauseTime,
				false
			);
		});

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		Delegate,
		MoveDuration,
		false
	);
}

void ALCAutoGimmick::AutoReturn()
{
	if (!bLoopingEnabled) return;
	if (!HasAuthority() || bIsMovingServer || bIsReturningServer || bIsRotatingServer || bIsReturningRotationServer)
		return;

	InitialLocation = GetActorLocation();
	TargetLocation = OriginalLocation;

	StartServerMovement(InitialLocation, TargetLocation, ReturnMoveDuration);
	Multicast_StartMovement(InitialLocation, TargetLocation, ReturnMoveDuration);

	const FQuat CurQuat = VisualMesh->GetComponentQuat();
	const FQuat OrigQuat = OriginalRotation.Quaternion();

	StartServerRotation(CurQuat, OrigQuat, ReturnRotationDuration);
	Multicast_StartRotation(CurQuat, OrigQuat, ReturnRotationDuration);

	FTimerDelegate ReturnDoneDelegate;
	ReturnDoneDelegate.BindLambda([this]()
		{
			SetActorLocation(TargetLocation);
			bIsReturningServer = false;
			bIsReturningRotationServer = false;

			TotalRotationIndex = 0;
			CurrentRotationQuat = OriginalRotationQuat;

			GetWorldTimerManager().SetTimer(
				AutoMoveTimerHandle,
				this,
				&ALCAutoGimmick::AutoMove,
				ReturnPauseTime,
				false
			);
		});

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		ReturnDoneDelegate,
		ReturnMoveDuration,
		false
	);
}

void ALCAutoGimmick::StopLoop()
{
	bLoopingEnabled = false;
	GetWorldTimerManager().ClearTimer(AutoMoveTimerHandle);
	GetWorldTimerManager().ClearTimer(AutoReturnTimerHandle);
	GetWorldTimerManager().ClearTimer(MovementTimerHandle);
	GetWorldTimerManager().ClearTimer(RotationTimerHandle);
	GetWorldTimerManager().ClearTimer(ReturnRotationTimerHandle);
	LOG_Art(Log, TEXT("[AutoGimmick] 루프 중지됨"));
}

void ALCAutoGimmick::ResumeLoop()
{
	if (!bLoopingEnabled)
	{
		bLoopingEnabled = true;
		StartLoop();
		LOG_Art(Log, TEXT("[AutoGimmick] 루프 재시작됨"));
	}
}