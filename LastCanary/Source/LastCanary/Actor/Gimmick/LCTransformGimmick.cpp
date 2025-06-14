#include "LCTransformGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCTransformGimmick::ALCTransformGimmick()
	: bUseAlternateToggle(false)
	, AlternateLocation(FVector::ZeroVector)
	, AlternateRotation(FRotator::ZeroRotator)
	, MoveVector(FVector::ZeroVector)
	, MoveDuration(1.f)
	, ReturnMoveDuration(1.f)
	, MoveIndex(0)
	, InitialLocation(FVector::ZeroVector)
	, TargetLocation(FVector::ZeroVector)
	, OriginalLocation(FVector::ZeroVector)
	, bIsMovingServer(false)
	, bIsReturningServer(false)
	, ClientStartLocation(FVector::ZeroVector)
	, ClientTargetLocation(FVector::ZeroVector)
	, ClientMoveDuration(0.f)
	, ClientMoveElapsed(0.f)
	, ServerMoveDuration(0.f)
	, ServerMoveElapsed(0.f)
	, RotationAxisEnum(EGimmickRotationAxis::Z)
	, RotationAngle(45.f)
	, RotationDuration(1.f)
	, ReturnRotationDuration(1.f)
	, RotationCount(1)
	, RotationIndex(0)
	, OriginalRotation(FRotator::ZeroRotator)
	, OriginalRotationQuat(FQuat::Identity)
	, InitialRotation(FQuat::Identity)
	, TargetRotation(FQuat::Identity)
	, TotalRotationIndex(0)
	, bIsRotatingServer(false)
	, bIsReturningRotationServer(false)
	, CurrentRotationQuat(FQuat::Identity)
	, ClientStartRotation(FQuat::Identity)
	, ClientTargetRotation(FQuat::Identity)
	, ClientRotationDuration(0.f)
	, ClientRotationElapsed(0.f)
	, ServerStartRotation(FQuat::Identity)
	, ServerTargetRotation(FQuat::Identity)
	, ServerRotationDuration(0.f)
	, ServerRotationElapsed(0.f)
	//, RotationDeltaQuat(FQuat::Identity)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);

}

#pragma region BeginPlay

void ALCTransformGimmick::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();
	OriginalRotation = VisualMesh->GetComponentRotation();
	OriginalRotationQuat = VisualMesh->GetComponentQuat();

	CurrentRotationQuat = VisualMesh->GetComponentQuat();

	if (bUseAlternateToggle)
	{
		AlternateLocation = OriginalLocation + MoveVector;
		const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
		FQuat OriginalQuat = OriginalRotation.Quaternion();
		FQuat RotatedQuat;

		switch (RotationAxisEnum)
		{
		case EGimmickRotationAxis::X:
			RotatedQuat = FQuat(FVector::RightVector, TotalAngleRad) * OriginalQuat;
			break;
		case EGimmickRotationAxis::Y:
			RotatedQuat = FQuat(FVector::UpVector, TotalAngleRad) * OriginalQuat;
			break;
		case EGimmickRotationAxis::Z:
		default:
			RotatedQuat = FQuat(FVector::ForwardVector, TotalAngleRad) * OriginalQuat;
			break;
		}

		AlternateRotation = RotatedQuat.Rotator();
	}
}


#pragma endregion

#pragma region Activation

void ALCTransformGimmick::ActivateGimmick_Implementation()
{
	if (!HasAuthority())
	{
		Server_ActivateGimmick();
		return;
	}

	if (bIsReturningServer || bIsMovingServer || bIsReturningRotationServer || bIsRotatingServer)
	{
		GetWorldTimerManager().ClearAllTimersForObject(this);
		bIsReturningServer = bIsMovingServer = bIsReturningRotationServer = bIsRotatingServer = false;
	}

	if (!ILCGimmickInterface::Execute_CanActivate(this)) return;

	Super::ActivateGimmick_Implementation();

	if (bUseAlternateToggle)
	{
		const FVector CurLoc = GetActorLocation();
		const FVector ToLoc = CurLoc.Equals(OriginalLocation, 1.f) ? AlternateLocation : OriginalLocation;

		const FQuat CurQuat = VisualMesh->GetComponentQuat();
		const FQuat OrigQuat = OriginalRotation.Quaternion();
		const FQuat AltQuat = AlternateRotation.Quaternion();
		const FQuat ToQuat = CurQuat.Equals(OrigQuat, 0.01f) ? AltQuat : OrigQuat;

		if (!CurLoc.Equals(ToLoc, 1.f))
		{
			StartServerMovement(CurLoc, ToLoc, MoveDuration);
			GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteMovement, MoveDuration, false);
			Multicast_StartMovement(CurLoc, ToLoc, MoveDuration);
		}

		if (!CurQuat.Equals(ToQuat, 0.01f))
		{
			StartServerRotation(CurQuat, ToQuat, RotationDuration);
			GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotation, RotationDuration, false);
			Multicast_StartRotation(CurQuat, ToQuat, RotationDuration);
		}
	}
	else
	{
		StartMovement();
		StartRotation();
	}
}

bool ALCTransformGimmick::CanActivate_Implementation()
{
	if (bIsMovingServer || bIsReturningServer || bIsRotatingServer || bIsReturningRotationServer)
	{
		LOG_Art_WARNING(TEXT("트랜스폼 기믹 ▶ 이동 또는 회전 중이라 CanActivate 거부됨"));
		return false;
	}
	return Super::CanActivate_Implementation();
}

void ALCTransformGimmick::ReturnToInitialState_Implementation()
{
	LOG_Art(Log, TEXT("▶ ReturnToInitialState 진입"));

	if (bIsReturningServer && bIsReturningRotationServer)
	{
		LOG_Art(Log, TEXT("▶ 이미 복귀 중 - ReturnToInitialState 무시"));
		return;
	}

	GetWorldTimerManager().ClearAllTimersForObject(this);
	bIsMovingServer = bIsRotatingServer = false;

	const FVector CurLoc = GetActorLocation();

	if (!CurLoc.Equals(OriginalLocation, 1.0f) && !bIsReturningServer)
	{
		bIsReturningServer = true;
		StartServerMovement(CurLoc, OriginalLocation, ReturnMoveDuration);
		GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteReturn, ReturnMoveDuration, false);
		Multicast_StartMovement(CurLoc, OriginalLocation, ReturnMoveDuration);
	}

	if (!bIsReturningRotationServer && TotalRotationIndex != 0)
	{
		bIsReturningRotationServer = true;

		const FQuat CurQuat = CurrentRotationQuat;
		const FQuat TargetQuat = OriginalRotationQuat;

		LOG_Art(Log, TEXT("▶ 회전 복귀 - 현재: %s → 목표(Original): %s"),
			*CurQuat.Rotator().ToCompactString(),
			*TargetQuat.Rotator().ToCompactString());

		StartServerRotation(CurQuat, TargetQuat, ReturnRotationDuration);
		GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotationReturn, ReturnRotationDuration, false);
		Multicast_StartRotation(CurQuat, TargetQuat, ReturnRotationDuration);
	}

}

FVector ALCTransformGimmick::GetRotationAxisVector(EGimmickRotationAxis AxisEnum) const
{
	switch (AxisEnum)
	{
	case EGimmickRotationAxis::X: return FVector::RightVector;
	case EGimmickRotationAxis::Y: return FVector::UpVector;
	case EGimmickRotationAxis::Z: return FVector::ForwardVector;
	default: return FVector::UpVector;
	}
}

#pragma endregion

#pragma region Movement - Server

void ALCTransformGimmick::StartMovement()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		return;
	}

	InitialLocation = GetActorLocation();
	MoveIndex++;

	const FVector Delta = MoveVector;
	TargetLocation = InitialLocation + Delta;

	bIsMovingServer = true;

	StartServerMovement(InitialLocation, TargetLocation, MoveDuration);

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&ALCTransformGimmick::CompleteMovement,
		MoveDuration,
		false
	);

	Multicast_StartMovement(InitialLocation, TargetLocation, MoveDuration);

	if (!IsRunningDedicatedServer())
	{
		StartClientMovement(InitialLocation, TargetLocation, MoveDuration);
	}
}

void ALCTransformGimmick::CompleteMovement()
{
	bIsMovingServer = false;
	SetActorLocation(TargetLocation);

	if (!bToggleState && !bUseAlternateToggle)
	{
		LOG_Art(Log, TEXT("▶ 상태 복귀 예약됨 - ReturnToInitialState %.1f초 후"), ReturnDelay);

		GetWorldTimerManager().SetTimer(ReturnMoveTimerHandle, [this]()
			{
				LOG_Art(Log, TEXT("▶ [이동] ReturnToInitialState 람다 호출됨"));
				this->ReturnToInitialState_Implementation();
			}, ReturnDelay, false);
	}
}

void ALCTransformGimmick::ReturnToInitialLocation()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		return;
	}

	bIsReturningServer = true;

	const FVector From = GetActorLocation();
	const FVector To = OriginalLocation;

	StartServerMovement(From, To, ReturnMoveDuration);

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&ALCTransformGimmick::CompleteReturn,
		ReturnMoveDuration,
		false
	);

	Multicast_StartMovement(From, To, ReturnMoveDuration);
}

void ALCTransformGimmick::CompleteReturn()
{
	bIsReturningServer = false;
	SetActorLocation(OriginalLocation);
	TargetLocation = OriginalLocation;
	MoveIndex = 0;
}

void ALCTransformGimmick::StartMovementToTarget(const FVector& NewTarget)
{
	if (bIsMovingServer || bIsReturningServer)
	{
		LOG_Art_WARNING(TEXT("▶ StartMovementToTarget - 이동 중으로 무시됨"));
		return;
	}

	InitialLocation = GetActorLocation();
	TargetLocation = NewTarget;

	GetWorldTimerManager().ClearTimer(ServerMoveTimer);
	GetWorldTimerManager().ClearTimer(MovementTimerHandle);

	bIsMovingServer = true;

	StartServerMovement(InitialLocation, TargetLocation, MoveDuration);

	Multicast_StartMovement(InitialLocation, TargetLocation, MoveDuration);

	if (!IsRunningDedicatedServer())
	{
		StartClientMovement(InitialLocation, TargetLocation, MoveDuration);
	}
}

#pragma endregion

#pragma region Movement - Interpolation

void ALCTransformGimmick::StartServerMovement(const FVector& From, const FVector& To, float Duration)
{
	ServerMoveElapsed = 0.f;
	ServerMoveDuration = Duration;
	InitialLocation = From;
	TargetLocation = To;

	GetWorldTimerManager().SetTimer(
		ServerMoveTimer,
		this,
		&ALCTransformGimmick::StepServerMovement,
		0.02f,
		true
	);
}

void ALCTransformGimmick::StepServerMovement()
{
	ServerMoveElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ServerMoveElapsed / ServerMoveDuration, 0.f, 1.f);
	const FVector NewLoc = FMath::Lerp(InitialLocation, TargetLocation, Alpha);

	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerMoveTimer);
		GetWorldTimerManager().ClearTimer(ReturnMoveTimerHandle);

		bIsReturningServer = false;
		bIsMovingServer = false;
		SetActorLocation(TargetLocation);
	}
}

void ALCTransformGimmick::Multicast_StartMovement_Implementation(const FVector& From, const FVector& To, float Duration)
{
	const FVector DeltaLocation = To - From;

	if (!HasAuthority())
	{
		StartClientMovement(From, To, Duration);
		StartClientAttachedMovement(DeltaLocation, Duration);
	}
	else
	{
		StartServerAttachedMovement(DeltaLocation, Duration);
	}
}

void ALCTransformGimmick::StartClientMovement(const FVector& From, const FVector& To, float Duration)
{
	ensure(!IsRunningDedicatedServer());

	ClientStartLocation = From;
	ClientTargetLocation = To;
	ClientMoveElapsed = 0.f;
	ClientMoveDuration = Duration;

	GetWorldTimerManager().SetTimer(
		ClientMoveTimer,
		this,
		&ALCTransformGimmick::StepClientMovement,
		0.02f,
		true
	);
}

void ALCTransformGimmick::StepClientMovement()
{
	ClientMoveElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ClientMoveElapsed / ClientMoveDuration, 0.f, 1.f);
	const FVector NewLoc = FMath::Lerp(ClientStartLocation, ClientTargetLocation, Alpha);

	SetActorLocation(NewLoc);

	if (Alpha >= 1.f || FMath::IsNearlyEqual(Alpha, 1.f, 0.01f))
	{
		GetWorldTimerManager().ClearTimer(ClientMoveTimer);
	}
}

void ALCTransformGimmick::StartServerAttachedMovement(const FVector& DeltaLocation, float Duration)
{
	for (AActor* Target : AttachedActors)
	{
		if (!IsValid(Target)) continue;

		if (AttachedMovementTimers.Contains(Target))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttachedMovementTimers[Target]);
			AttachedMovementTimers.Remove(Target);
		}

		const FVector StartLoc = Target->GetActorLocation();
		const FVector EndLoc = StartLoc + DeltaLocation;
		TSharedPtr<float> Elapsed = MakeShared<float>(0.f);

		FTimerDelegate Delegate;
		Delegate.BindLambda([=]()
			{
				if (!IsValid(Target)) return;
				*Elapsed += 0.02f;
				const float Alpha = FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
				Target->SetActorLocation(FMath::Lerp(StartLoc, EndLoc, Alpha));
			});

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
		AttachedMovementTimers.Add(Target, Handle);
	}
}

void ALCTransformGimmick::StartClientAttachedMovement(const FVector& DeltaLocation, float Duration)
{
	if (HasAuthority()) return;

	for (AActor* Target : AttachedActors)
	{
		if (!IsValid(Target)) continue;

		if (AttachedMovementTimers.Contains(Target))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttachedMovementTimers[Target]);
			AttachedMovementTimers.Remove(Target);
		}

		const FVector StartLoc = Target->GetActorLocation();
		const FVector EndLoc = StartLoc + DeltaLocation;
		TSharedPtr<float> Elapsed = MakeShared<float>(0.f);

		FTimerDelegate Delegate;
		Delegate.BindLambda([=]()
			{
				if (!IsValid(Target)) return;
				*Elapsed += 0.02f;
				const float Alpha = FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
				Target->SetActorLocation(FMath::Lerp(StartLoc, EndLoc, Alpha));
			});

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
		AttachedMovementTimers.Add(Target, Handle);
	}
}

#pragma endregion

#pragma region Rotation - Server

void ALCTransformGimmick::StartRotation()
{
	if (bIsRotatingServer || bIsReturningRotationServer)
	{
		return;
	}

	InitialRotation = CurrentRotationQuat;

	FVector Axis = GetRotationAxisVector(RotationAxisEnum);
	const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
	const FQuat DeltaQuat = FQuat(Axis, TotalAngleRad);

	TargetRotation = DeltaQuat * InitialRotation;

	// ✅ 회전 누적
	CurrentRotationQuat = TargetRotation;
	TotalRotationIndex += RotationCount;

	LastRotationStartQuat = VisualMesh->GetComponentQuat();
	LastRotationDeltaQuat = DeltaQuat;

	LOG_Art(Log, TEXT("▶ StartRotation - Axis: %s | AngleDeg: %.1f | FromQuat: %s | ToQuat: %s"),
		*Axis.ToString(),
		RotationAngle * RotationCount,
		*InitialRotation.Rotator().ToCompactString(),
		*TargetRotation.Rotator().ToCompactString()
	);

	StartServerRotation(InitialRotation, TargetRotation, RotationDuration);
	Multicast_StartRotation(InitialRotation, TargetRotation, RotationDuration);

	if (!IsRunningDedicatedServer())
	{
		StartClientRotation(InitialRotation, TargetRotation, RotationDuration);
	}
}

void ALCTransformGimmick::CompleteRotation()
{
	bIsRotatingServer = false;

	VisualMesh->SetWorldRotation(TargetRotation);

	CurrentRotationQuat = TargetRotation;

	if (!bToggleState && !bUseAlternateToggle)
	{
		LOG_Art(Log, TEXT("▶ 회전 복귀 예약됨 - ReturnToInitialState %.1f초 후"), ReturnDelay);

		GetWorldTimerManager().SetTimer(ReturnRotationTimerHandle, [this]()
			{
				LOG_Art(Log, TEXT("▶ [회전] ReturnToInitialState 람다 호출됨"));
				this->ReturnToInitialState_Implementation();
			}, ReturnDelay, false);
	}
}

void ALCTransformGimmick::ReturnToInitialRotation()
{
	if (bIsRotatingServer || bIsReturningRotationServer)
	{
		return;
	}

	bIsReturningRotationServer = true;

	InitialRotation = CurrentRotationQuat;
	TargetRotation = OriginalRotationQuat;

	const FQuat DeltaQuat = TargetRotation * InitialRotation.Inverse();

	float AngleRad;
	FVector Axis;
	DeltaQuat.ToAxisAndAngle(Axis, AngleRad);

	RotationAngle = FMath::RadiansToDegrees(AngleRad);
	RotationCount = 1;

	LOG_Art(Log, TEXT("▶ 복귀 회전 계산 - Axis: %s | AngleDeg: %.1f | From: %s | To: %s"),
		*Axis.ToString(),
		RotationAngle,
		*InitialRotation.Rotator().ToCompactString(),
		*TargetRotation.Rotator().ToCompactString()
	);

	StartServerRotation(InitialRotation, TargetRotation, ReturnRotationDuration);
	Multicast_StartRotation(InitialRotation, TargetRotation, ReturnRotationDuration);

	if (!IsRunningDedicatedServer())
	{
		StartClientRotation(InitialRotation, TargetRotation, ReturnRotationDuration);
	}
}

void ALCTransformGimmick::CompleteRotationReturn()
{
	bIsReturningRotationServer = false;
	bIsRotatingServer = false;

	VisualMesh->SetWorldRotation(OriginalRotation);
	RotationIndex = 0;
	TotalRotationIndex = 0;

}

#pragma endregion

#pragma region Rotation - Interpolation

void ALCTransformGimmick::StartServerRotation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration)
{
	if (bIsRotatingServer)
	{
		return;
	}

	bIsRotatingServer = true;
	ServerRotationElapsed = 0.f;
	ServerRotationDuration = Duration;

	ServerStartRotation = FromQuat;
	ServerTargetRotation = ToQuat;

	LOG_Art(Log, TEXT("▶ StartServerRotation - From: %s | To: %s | Duration: %.2fs"),
		*FromQuat.Rotator().ToCompactString(),
		*ToQuat.Rotator().ToCompactString(),
		Duration
	);

	GetWorldTimerManager().SetTimer(
		ServerRotationTimer,
		this,
		&ALCTransformGimmick::StepServerRotation,
		0.02f,
		true
	);
}

void ALCTransformGimmick::StepServerRotation()
{
	if (!bIsRotatingServer)
	{
		return;
	}

	ServerRotationElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ServerRotationElapsed / ServerRotationDuration, 0.f, 1.f);

	const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
	const float StepAngleRad = FMath::Lerp(0.f, TotalAngleRad, Alpha);

	FVector Axis;
	switch (RotationAxisEnum)
	{
	case EGimmickRotationAxis::X: Axis = FVector::RightVector;   break;
	case EGimmickRotationAxis::Y: Axis = FVector::UpVector;      break;
	case EGimmickRotationAxis::Z: Axis = FVector::ForwardVector; break;
	default:                    Axis = FVector::ForwardVector;   break;
	}

	const FQuat NewQuat = FQuat(Axis, StepAngleRad) * InitialRotation;
	VisualMesh->SetWorldRotation(NewQuat);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerRotationTimer);
		CompleteRotation();
	}
}

void ALCTransformGimmick::Multicast_StartRotation_Implementation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration)
{
	if (!HasAuthority())
	{
		StartClientRotation(FromQuat, ToQuat, Duration);
		StartClientAttachedRotation(FromQuat, ToQuat, Duration);
	}
	else
	{
		StartServerAttachedRotation(FromQuat, ToQuat, Duration);
	}
}

void ALCTransformGimmick::StartClientRotation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration)
{
	ensure(!IsRunningDedicatedServer());

	VisualMesh->SetWorldRotation(FromQuat);

	ClientStartRotation = FromQuat;
	ClientTargetRotation = ToQuat;
	ClientRotationDuration = Duration;
	ClientRotationElapsed = 0.f;

	LOG_Art(Log, TEXT("▶ StartClientRotation - From: %s | To: %s | Duration: %.2fs"),
		*FromQuat.Rotator().ToCompactString(),
		*ToQuat.Rotator().ToCompactString(),
		Duration
	);

	GetWorldTimerManager().SetTimer(
		ClientRotationTimer,
		this,
		&ALCTransformGimmick::StepClientRotation,
		0.02f,
		true
	);
}

void ALCTransformGimmick::StepClientRotation()
{
	ClientRotationElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ClientRotationElapsed / ClientRotationDuration, 0.f, 1.f);

	const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
	const float StepAngleRad = FMath::Lerp(0.f, TotalAngleRad, Alpha);

	FVector Axis;
	switch (RotationAxisEnum)
	{
	case EGimmickRotationAxis::X: Axis = FVector::RightVector;   break;
	case EGimmickRotationAxis::Y: Axis = FVector::UpVector;      break;
	case EGimmickRotationAxis::Z: Axis = FVector::ForwardVector; break;
	default:                    Axis = FVector::ForwardVector;   break;
	}

	const FQuat NewQuat = FQuat(Axis, StepAngleRad) * ClientStartRotation;
	VisualMesh->SetWorldRotation(NewQuat);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ClientRotationTimer);
	}
}

void ALCTransformGimmick::StartServerAttachedRotation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration)
{
	for (AActor* AttachedActor : AttachedActors)
	{
		if (!IsValid(AttachedActor))
			continue;

		FTimerHandle& Handle = AttachedRotationTimers.FindOrAdd(AttachedActor);
		TSharedPtr<float> ElapsedTime = MakeShared<float>(0.f);

		FTimerDelegate Delegate;
		Delegate.BindLambda([Actor = AttachedActor, FromQuat, ToQuat, Duration, ElapsedTime]()
			{
				if (!IsValid(Actor))
					return;

				*ElapsedTime += 0.02f;
				const float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.f, 1.f);
				const FQuat NewQuat = FQuat::SlerpFullPath(FromQuat, ToQuat, Alpha);
				Actor->SetActorRotation(NewQuat);
			});

		GetWorldTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
	}
}

void ALCTransformGimmick::StartClientAttachedRotation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration)
{
	for (AActor* AttachedActor : AttachedActors)
	{
		if (!IsValid(AttachedActor))
			continue;

		FTimerHandle& Handle = AttachedRotationTimers.FindOrAdd(AttachedActor);
		TSharedPtr<float> ElapsedTime = MakeShared<float>(0.f);

		FTimerDelegate Delegate;
		Delegate.BindLambda([Actor = AttachedActor, FromQuat, ToQuat, Duration, ElapsedTime]()
			{
				if (!IsValid(Actor))
					return;

				*ElapsedTime += 0.02f;
				const float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.f, 1.f);
				const FQuat NewQuat = FQuat::SlerpFullPath(FromQuat, ToQuat, Alpha);
				Actor->SetActorRotation(NewQuat);
			});

		GetWorldTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
	}
}

#pragma endregion
