#include "LCTransformGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCTransformGimmick::ALCTransformGimmick()
	: MoveVector(FVector::ZeroVector)
	, MoveDuration(1.f)
	, ReturnMoveDuration(1.f)
	, MoveIndex(0)
	, bIsMovingServer(false)
	, bIsReturningServer(false)
	, RotationAxis(FVector::ZeroVector)
	, AccumulatedDeltaRotation(FRotator::ZeroRotator)
	, RotationDuration(1.f)
	, ReturnRotationDuration(1.f)
	, RotationIndex(0)
	, bIsRotatingServer(false)
	, bIsReturningRotationServer(false)
	, ServerMoveElapsed(0.f)
	, ServerMoveDuration(0.f)
	, ClientMoveElapsed(0.f)
	, ClientMoveDuration(0.f)
	, ServerRotationElapsed(0.f)
	, ServerRotationDuration(0.f)
	, ClientRotationElapsed(0.f)
	, ClientRotationDuration(0.f)
	, ServerStartRotation(FRotator::ZeroRotator)
	, ServerTargetRotation(FRotator::ZeroRotator)
	, ClientStartRotation(FRotator::ZeroRotator)
	, ClientTargetRotation(FRotator::ZeroRotator)
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

	if (bUseAlternateToggle)
	{
		AlternateLocation = OriginalLocation + MoveVector;
		AlternateRotation = OriginalRotation + FRotator(RotationAxis.X, RotationAxis.Y, RotationAxis.Z);
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

		const FRotator CurRot = VisualMesh->GetComponentRotation();
		const FRotator ToRot = CurRot.Equals(OriginalRotation, 1.f) ? AlternateRotation : OriginalRotation;

		if (!CurLoc.Equals(ToLoc, 1.f))
		{
			StartServerMovement(CurLoc, ToLoc, MoveDuration);
			GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteMovement, MoveDuration, false);
			Multicast_StartMovement(CurLoc, ToLoc, MoveDuration);
		}

		if (!CurRot.Equals(ToRot, 1.f))
		{
			StartServerRotation(CurRot, ToRot, RotationDuration);
			GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotation, RotationDuration, false);
			Multicast_StartRotation(CurRot, ToRot, RotationDuration);
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
	const FRotator CurRot = VisualMesh->GetComponentRotation();

	if (!CurLoc.Equals(OriginalLocation, 1.0f) && !bIsReturningServer)
	{
		bIsReturningServer = true;
		StartServerMovement(CurLoc, OriginalLocation, ReturnMoveDuration);
		GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteReturn, ReturnMoveDuration, false);
		Multicast_StartMovement(CurLoc, OriginalLocation, ReturnMoveDuration);
	}

	if (!CurRot.Equals(OriginalRotation, 1.0f) && !bIsReturningRotationServer)
	{
		bIsReturningRotationServer = true;
		StartServerRotation(CurRot, OriginalRotation, ReturnRotationDuration);
		GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotationReturn, ReturnRotationDuration, false);
		Multicast_StartRotation(CurRot, OriginalRotation, ReturnRotationDuration);
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

	InitialRotation = VisualMesh->GetComponentRotation();

	RotationIndex++;
	const FRotator DeltaRot = FRotator(RotationAxis.X, RotationAxis.Y, RotationAxis.Z);
	TargetRotation = InitialRotation + DeltaRot;

	bIsRotatingServer = true;

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCTransformGimmick::CompleteRotation,
		RotationDuration,
		false
	);

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

	const FRotator From = VisualMesh->GetComponentRotation();
	const FRotator To = OriginalRotation;

	StartServerRotation(From, To, ReturnRotationDuration);

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCTransformGimmick::CompleteRotationReturn,
		ReturnRotationDuration,
		false
	);

	Multicast_StartRotation(From, To, ReturnRotationDuration);
}

void ALCTransformGimmick::CompleteRotationReturn()
{
	bIsReturningRotationServer = false;
	bIsRotatingServer = false;

	VisualMesh->SetWorldRotation(OriginalRotation);
	RotationIndex = 0;
}

#pragma endregion

#pragma region Rotation - Interpolation

void ALCTransformGimmick::StartServerRotation(const FRotator& From, const FRotator& To, float Duration)
{
	ServerRotationElapsed = 0.f;
	ServerRotationDuration = Duration;
	ServerStartRotation = From;
	ServerTargetRotation = To;

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
	ServerRotationElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ServerRotationElapsed / ServerRotationDuration, 0.f, 1.f);
	const FRotator NewRot = FMath::Lerp(ServerStartRotation, ServerTargetRotation, Alpha);
	VisualMesh->SetWorldRotation(NewRot);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerRotationTimer);
		GetWorldTimerManager().ClearTimer(ReturnMoveTimerHandle);

		bIsReturningRotationServer = false;
		bIsRotatingServer = false;

		VisualMesh->SetWorldRotation(ServerTargetRotation);
	}
}

void ALCTransformGimmick::Multicast_StartRotation_Implementation(const FRotator& From, const FRotator& To, float Duration)
{
	if (!HasAuthority())
	{
		StartClientRotation(From, To, Duration);

		const FRotator DeltaRot = To - From;
		StartClientAttachedRotation(DeltaRot, Duration);
	}
	else
	{
		const FRotator DeltaRot = To - From;
		StartServerAttachedRotation(DeltaRot, Duration);
	}
}

void ALCTransformGimmick::StartClientRotation(const FRotator& From, const FRotator& To, float Duration)
{
	ensure(!IsRunningDedicatedServer());

	ClientRotationElapsed = 0.f;
	ClientRotationDuration = Duration;
	ClientStartRotation = From;
	ClientTargetRotation = To;

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
	const FRotator NewRot = FMath::Lerp(ClientStartRotation, ClientTargetRotation, Alpha);
	VisualMesh->SetWorldRotation(NewRot);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ClientRotationTimer);
	}
}

void ALCTransformGimmick::StartServerAttachedRotation(const FRotator& DeltaRot, float Duration)
{
	for (AActor* Target : AttachedActors)
	{
		if (!IsValid(Target)) continue;

		if (AttachedRotationTimers.Contains(Target))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttachedRotationTimers[Target]);
			AttachedRotationTimers.Remove(Target);
		}

		const FRotator StartRot = Target->GetActorRotation();
		const FRotator EndRot = StartRot + DeltaRot;
		TSharedPtr<float> ElapsedTime = MakeShared<float>(0.f);

		FTimerHandle Handle;
		FTimerDelegate Delegate;
		Delegate.BindLambda([Target, ElapsedTime, StartRot, EndRot, Duration]()
			{
				if (!IsValid(Target)) return;

				*ElapsedTime += 0.02f;
				const float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.f, 1.f);
				const FRotator NewRot = FMath::Lerp(StartRot, EndRot, Alpha);
				Target->SetActorRotation(NewRot);
			});

		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
		AttachedRotationTimers.Add(Target, Handle);
	}
}

void ALCTransformGimmick::StartClientAttachedRotation(const FRotator& DeltaRot, float Duration)
{
	if (HasAuthority()) return;

	for (AActor* Target : AttachedActors)
	{
		if (!IsValid(Target)) continue;

		if (AttachedRotationTimers.Contains(Target))
		{
			GetWorld()->GetTimerManager().ClearTimer(AttachedRotationTimers[Target]);
			AttachedRotationTimers.Remove(Target);
		}

		const FRotator StartRot = Target->GetActorRotation();
		const FRotator EndRot = StartRot + DeltaRot;
		TSharedPtr<float> ElapsedTime = MakeShared<float>(0.f);

		FTimerHandle Handle;
		FTimerDelegate Delegate;
		Delegate.BindLambda([Target, ElapsedTime, StartRot, EndRot, Duration]()
			{
				if (!IsValid(Target)) return;

				*ElapsedTime += 0.02f;
				const float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.f, 1.f);
				const FRotator NewRot = FMath::Lerp(StartRot, EndRot, Alpha);
				Target->SetActorRotation(NewRot);
			});

		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
		AttachedRotationTimers.Add(Target, Handle);
	}
}

#pragma endregion

