#include "LCTransformGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCTransformGimmick::ALCTransformGimmick()
	: MoveStep(100.f)
	, MoveVector(FVector::ZeroVector)
	, bUseAxis(false)
	, MoveSpeed(100.f)
	, MoveIndex(0)
	, bIsMovingServer(false)
	, bIsReturningServer(false)
	, RotationStep(45.f)
	, RotationAxis(FVector::ZeroVector)
	, RotationSpeed(90.f)
	, RotationIndex(0)
	, bIsRotatingServer(false)
	, bIsReturningRotationServer(false)
	, ClientMoveDuration(0.f)
	, ClientMoveElapsed(0.f)
	, ClientRotationDuration(0.f)
	, ClientRotationElapsed(0.f)
	, ServerMoveDuration(0.f)
	, ServerMoveElapsed(0.f)
	, ServerRotationDuration(0.f)
	, ServerRotationElapsed(0.f)
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
		const FVector DeltaLoc = bUseAxis ? MoveVector : FVector(0.f, 0.f, MoveStep);
		AlternateLocation = OriginalLocation + DeltaLoc;

		const FRotator DeltaRot = bUseAxis ? FRotator(MoveVector.Y, MoveVector.Z, MoveVector.X) : FRotator(0.f, RotationStep, 0.f);
		AlternateRotation = OriginalRotation + DeltaRot;
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
		// 보간 중이면 초기화
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

		const float MoveDuration = FVector::Dist(CurLoc, ToLoc) / MoveSpeed;
		const float RotDuration = (ToRot - CurRot).GetManhattanDistance(FRotator::ZeroRotator) / RotationSpeed;

		if (!CurLoc.Equals(ToLoc, 1.f))
		{
			StartServerMovement(CurLoc, ToLoc, MoveDuration);
			GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteMovement, MoveDuration, false);
			Multicast_StartMovement(CurLoc, ToLoc, MoveDuration);
		}

		if (!CurRot.Equals(ToRot, 1.f))
		{
			StartServerRotation(CurRot, ToRot, RotDuration);
			GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotation, RotDuration, false);
			Multicast_StartRotation(CurRot, ToRot, RotDuration);
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
	if (bIsReturningServer || bIsReturningRotationServer)
	{
		LOG_Art(Log, TEXT("▶ 이미 복귀 중 - ReturnToInitialState 무시"));
		return;
	}

	// 이동/회전 중이면 정지
	GetWorldTimerManager().ClearAllTimersForObject(this);
	bIsMovingServer = bIsRotatingServer = false;

	const FVector CurLoc = GetActorLocation();
	const FVector ToLoc = OriginalLocation;
	const FRotator CurRot = VisualMesh->GetComponentRotation();
	const FRotator ToRot = OriginalRotation;

	if (!CurLoc.Equals(ToLoc, 1.0f))
	{
		const float Duration = FVector::Dist(CurLoc, ToLoc) / MoveSpeed;
		bIsReturningServer = true;
		StartServerMovement(CurLoc, ToLoc, Duration);
		GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteReturn, Duration, false);
		Multicast_StartMovement(CurLoc, ToLoc, Duration);
	}

	if (!CurRot.Equals(ToRot, 1.0f))
	{
		const float Duration = (ToRot - CurRot).GetManhattanDistance(FRotator::ZeroRotator) / RotationSpeed;
		bIsReturningRotationServer = true;
		StartServerRotation(CurRot, ToRot, Duration);
		GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotationReturn, Duration, false);
		Multicast_StartRotation(CurRot, ToRot, Duration);
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

	const FVector Delta = bUseAxis ? MoveVector : FVector(MoveStep, 0.f, 0.f);
	TargetLocation = InitialLocation + Delta;

	const float Distance = Delta.Size();
	const float Duration = Distance / MoveSpeed;

	bIsMovingServer = true;

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&ALCTransformGimmick::CompleteMovement,
		Duration,
		false
	);

	Multicast_StartMovement(InitialLocation, TargetLocation, Duration);

	if (!IsRunningDedicatedServer())
	{
		StartClientMovement(InitialLocation, TargetLocation, Duration);
	}
}

void ALCTransformGimmick::CompleteMovement()
{
	bIsMovingServer = false;
	SetActorLocation(TargetLocation);

	if (!bToggleState)
	{
		GetWorldTimerManager().SetTimer(
			ReturnTimerHandle,
			this,
			&ALCTransformGimmick::ReturnToInitialLocation,
			ReturnDelay,
			false
		);
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
	const float Duration = FVector::Dist(From, To) / MoveSpeed;

	StartServerMovement(From, To, Duration);

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&ALCTransformGimmick::CompleteReturn,
		Duration,
		false
	);

	Multicast_StartMovement(From, To, Duration);
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

	const float Distance = FVector::Dist(InitialLocation, TargetLocation);
	const float Duration = Distance / MoveSpeed;

	GetWorldTimerManager().ClearTimer(ServerMoveTimer);
	GetWorldTimerManager().ClearTimer(MovementTimerHandle);

	bIsMovingServer = true;

	StartServerMovement(InitialLocation, TargetLocation, Duration);

	Multicast_StartMovement(InitialLocation, TargetLocation, Duration);

	if (!IsRunningDedicatedServer())
	{
		StartClientMovement(InitialLocation, TargetLocation, Duration);
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
	const float DeltaTime = GetWorld()->GetDeltaSeconds(); 
	ServerMoveElapsed += DeltaTime;

	const float Alpha = FMath::Clamp(ServerMoveElapsed / ServerMoveDuration, 0.f, 1.f);
	const FVector NewLoc = FMath::Lerp(InitialLocation, TargetLocation, Alpha);
	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerMoveTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

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

	const FRotator DeltaRot = bUseAxis
		? FRotator(RotationAxis.X, RotationAxis.Y, RotationAxis.Z)
		: FRotator(0.f, RotationStep, 0.f);

	if (DeltaRot.IsNearlyZero())
	{
		LOG_Art_WARNING(TEXT("▶ 회전량이 0이라 StartRotation 생략"));
		return;
	}

	TargetRotation = InitialRotation + DeltaRot;
	const float Duration = DeltaRot.GetManhattanDistance(FRotator::ZeroRotator) / RotationSpeed;

	bIsRotatingServer = true;

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCTransformGimmick::CompleteRotation,
		Duration,
		false
	);

	Multicast_StartRotation(InitialRotation, TargetRotation, Duration);

	if (!IsRunningDedicatedServer())
	{
		StartClientRotation(InitialRotation, TargetRotation, Duration);
	}
}

void ALCTransformGimmick::CompleteRotation()
{
	bIsRotatingServer = false;
	VisualMesh->SetWorldRotation(TargetRotation);

	if (!bToggleState)
	{
		GetWorldTimerManager().SetTimer(
			ReturnTimerHandle,
			this,
			&ALCTransformGimmick::ReturnToInitialRotation,
			ReturnDelay,
			false
		);
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
	const float Duration = (To - From).GetManhattanDistance(FRotator::ZeroRotator) / RotationSpeed;

	StartServerRotation(From, To, Duration);

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCTransformGimmick::CompleteReturn,
		Duration,
		false
	);

	Multicast_StartRotation(From, To, Duration);
}

void ALCTransformGimmick::CompleteRotationReturn()
{
	bIsReturningRotationServer = false;
	bIsRotatingServer = false;

	VisualMesh->SetWorldRotation(OriginalRotation);
	TargetRotation = OriginalRotation;
	RotationIndex = 0;
}

#pragma endregion

#pragma region Rotation - Interpolation

void ALCTransformGimmick::StartServerRotation(const FRotator& From, const FRotator& To, float Duration)
{
	ServerRotationElapsed = 0.f;
	ServerRotationDuration = Duration;
	InitialRotation = From;
	TargetRotation = To;

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
	const FRotator NewRot = FMath::Lerp(InitialRotation, TargetRotation, Alpha);

	VisualMesh->SetWorldRotation(NewRot);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerRotationTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

		bIsReturningRotationServer = false;
		VisualMesh->SetWorldRotation(TargetRotation);
	}
}

void ALCTransformGimmick::Multicast_StartRotation_Implementation(const FRotator& From, const FRotator& To, float Duration)
{
	const FRotator DeltaRot = To - From;

	if (!HasAuthority())
	{
		StartClientRotation(From, To, Duration);
		StartClientAttachedRotation(DeltaRot, Duration);
	}
	else
	{
		StartServerRotation(From, To, Duration);
		StartServerAttachedRotation(DeltaRot, Duration);
	}
}


void ALCTransformGimmick::StartClientRotation(const FRotator& From, const FRotator& To, float Duration)
{
	ensure(!IsRunningDedicatedServer());

	ClientStartRotation = From;
	ClientTargetRotation = To;
	ClientRotationElapsed = 0.f;
	ClientRotationDuration = Duration;

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
		TSharedPtr<float> Elapsed = MakeShared<float>(0.f);

		FTimerDelegate Delegate;
		Delegate.BindLambda([=]()
			{
				if (!IsValid(Target)) return;
				*Elapsed += 0.02f;
				const float Alpha = FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
				Target->SetActorRotation(FMath::Lerp(StartRot, EndRot, Alpha));
			});

		FTimerHandle Handle;
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
		TSharedPtr<float> Elapsed = MakeShared<float>(0.f);

		FTimerDelegate Delegate;
		Delegate.BindLambda([=]()
			{
				if (!IsValid(Target)) return;
				*Elapsed += 0.02f;
				const float Alpha = FMath::Clamp(*Elapsed / Duration, 0.f, 1.f);
				Target->SetActorRotation(FMath::Lerp(StartRot, EndRot, Alpha));
			});

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);
		AttachedRotationTimers.Add(Target, Handle);
	}
}

#pragma endregion
