#include "LCTransformGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "Component/AttachedSyncComponent.h"
#include "LastCanary.h"

ALCTransformGimmick::ALCTransformGimmick()
	: bUseAlternateToggle(false)
	, AlternateLocation(FVector::ZeroVector)
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
	, ReturnRotationDeltaQuat(FQuat::Identity)
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
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

}

#pragma region BeginPlay

void ALCTransformGimmick::BeginPlay()
{
	Super::BeginPlay();

	OriginalLocation = GetActorLocation();

	OriginalRotation = VisualMesh->GetComponentRotation();
	OriginalRotationQuat = VisualMesh->GetComponentQuat();

	CurrentRotationQuat = OriginalRotationQuat;

	//LOG_Art(Log, TEXT(" BeginPlay : 초기 위치: %s | 초기 회전: %s"),
	//	*OriginalLocation.ToCompactString(),
	//	*OriginalRotation.ToCompactString());

	/*LOG_Art(Log, TEXT(" BeginPlay 호출 테스트 로그"));
	LOG_Art_WARNING(TEXT("BeginPlay 호출 테스트 경고"));
	LOG_Art_ERROR(TEXT("BeginPlay 호출 테스트 에러"));

	CHECK_Art(false, ); */

	if (bUseAlternateToggle)
	{
		if (AlternateLocation.IsNearlyZero())
		{
			AlternateLocation = OriginalLocation + MoveVector;
		}

		if (GetAlternateRotationQuat().Equals(OriginalRotationQuat, 0.01f))
		{
			//LOG_Art_WARNING(TEXT("AlternateRotation이 OriginalRotation과 동일함"));
		}
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

		CacheOriginalRotation();

	if (bUseAlternateToggle)
	{
		const FVector CurLoc = GetActorLocation();
		const FVector ToLoc = CurLoc.Equals(OriginalLocation, 1.f) ? AlternateLocation : OriginalLocation;

		const FQuat CurQuat = VisualMesh->GetComponentQuat();
		const FQuat OrigQuat = OriginalRotation.Quaternion();
		const FQuat AltQuat = GetAlternateRotationQuat();
		const FQuat ToQuat = CurQuat.Equals(OrigQuat, 0.01f) ? AltQuat : OrigQuat;

		if (!CurLoc.Equals(ToLoc, 1.f))
		{
			StartServerMovement(CurLoc, ToLoc, MoveDuration);
			GetWorldTimerManager().SetTimer(MovementTimerHandle, this, &ALCTransformGimmick::CompleteMovement, MoveDuration, false);
			Multicast_StartMovement(CurLoc, ToLoc, MoveDuration);
		}

		if (!CurQuat.Equals(ToQuat, 0.01f))
		{
			const bool bIsReturnRotation = ToQuat.Equals(OriginalRotationQuat, 0.01f); 

			StartServerRotation(CurQuat, ToQuat, RotationDuration);
			GetWorldTimerManager().SetTimer(RotationTimerHandle, this, &ALCTransformGimmick::CompleteRotation, RotationDuration, false);
			Multicast_StartRotation(CurQuat, ToQuat, RotationDuration, bIsReturnRotation);
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
		//LOG_Art_WARNING(TEXT("트랜스폼 기믹 ▶ 이동 또는 회전 중이라 CanActivate 거부됨"));
		return false;
	}
	return Super::CanActivate_Implementation();
}

void ALCTransformGimmick::ReturnToInitialState_Implementation()
{
	//LOG_Art(Log, TEXT("▶ ReturnToInitialState 진입"));

	if (bIsReturningServer && bIsReturningRotationServer)
	{
		//LOG_Art(Log, TEXT(" 이미 복귀 중 - ReturnToInitialState 무시"));
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
		//LOG_Art(Log, TEXT(" 회전 복귀 조건 만족 → StartReturnRotation 진입"));

		CurrentRotationQuat = VisualMesh->GetComponentQuat();

		StartReturnRotation();

		bIsReturningRotationServer = true;

	}
	else
	{
		////LOG_Art(Log, TEXT("❌ 회전 복귀 조건 불충족 - bIsReturningRotationServer: %d | TotalRotationIndex: %d"),
		//	bIsReturningRotationServer, TotalRotationIndex);
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

	//const FVector Delta = GetActorTransform().TransformVectorNoScale(MoveVector);
	//TargetLocation = InitialLocation + Delta;
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
		//LOG_Art(Log, TEXT("▶ 상태 복귀 예약됨 - ReturnToInitialState %.1f초 후"), ReturnDelay);

		GetWorldTimerManager().SetTimer(ReturnMoveTimerHandle, [this]()
			{
				//LOG_Art(Log, TEXT("▶ [이동] ReturnToInitialState 람다 호출됨"));
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
		//LOG_Art_WARNING(TEXT("▶ StartMovementToTarget - 이동 중으로 무시됨"));
		return;
	}

	InitialLocation = GetActorLocation();
	TargetLocation = NewTarget;

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
	SetActorLocation(From);

	Multicast_StartMovement(From, To, Duration);

	if (AttachedSyncComponent)
	{
		AttachedSyncComponent->BroadcastStartMovement(From, To, Duration);
	}
}

void ALCTransformGimmick::Multicast_StartMovement_Implementation(const FVector& From, const FVector& To, float Duration)
{
	const FVector DeltaLocation = To - From;

	if (GetNetMode() != NM_DedicatedServer)
	{
		StartClientMovement(From, To, Duration);
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

#pragma endregion

#pragma region Rotation - Server

void ALCTransformGimmick::StartRotation()
{
	CacheOriginalRotation();

	if (bIsRotatingServer || bIsReturningRotationServer)
	{
		return;
	}

	OriginalRotationQuat = VisualMesh->GetComponentQuat();
	OriginalRotation = VisualMesh->GetComponentRotation();

	InitialRotation = CurrentRotationQuat;

	const FVector Axis = GetRotationAxisVector(RotationAxisEnum);
	const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
	const FQuat DeltaQuat = FQuat(Axis, TotalAngleRad);

	TargetRotation = DeltaQuat * InitialRotation;
	CurrentRotationQuat = TargetRotation;
	TotalRotationIndex += RotationCount;

	////LOG_Art(Log, TEXT(" StartRotation - Axis: %s | Angle: %.1f | From: %s | To: %s"),
	//	*Axis.ToString(),
	//	RotationAngle * RotationCount,
	//	*InitialRotation.Rotator().ToCompactString(),
	//	*TargetRotation.Rotator().ToCompactString()
	//);

	Multicast_StartRotation(InitialRotation, TargetRotation, RotationDuration, false); 

	if (GetNetMode() != NM_DedicatedServer)
	{
		StartClientRotation(InitialRotation, TargetRotation, RotationDuration, false);
	}

	if (UAttachedSyncComponent* Sync = FindComponentByClass<UAttachedSyncComponent>())
	{
		Sync->BroadcastStartRotation(InitialRotation, TargetRotation, RotationDuration);
	}
	else
	{
		//LOG_Art_WARNING(TEXT("회전 시작 시 Sync 컴포넌트 찾기 실패 (FindComponentByClass)"));
	}
}

void ALCTransformGimmick::CompleteRotation()
{
	bIsRotatingServer = false;

	if (!bIsReturningRotationClient)
	{
		VisualMesh->SetWorldRotation(TargetRotation);
		CurrentRotationQuat = TargetRotation;
	}

	if (!bToggleState && !bUseAlternateToggle)
	{
		//LOG_Art(Log, TEXT("▶ 회전 복귀 예약됨 - ReturnToInitialState %.1f초 후"), ReturnDelay);

		GetWorldTimerManager().SetTimer(ReturnRotationTimerHandle, [this]()
			{
				//LOG_Art(Log, TEXT("▶ [회전] ReturnToInitialState 람다 호출됨"));
				this->ReturnToInitialState_Implementation();
			}, ReturnDelay, false);
	}
}

void ALCTransformGimmick::StartReturnRotation()
{
	if (bIsRotatingServer || bIsReturningRotationServer)
	{
		return;
	}

	bIsReturningRotationServer = true;

	InitialRotation = CurrentRotationQuat;
	TargetRotation = OriginalRotationQuat;

	CurrentRotationQuat = TargetRotation;

	////LOG_Art(Log, TEXT("▶ 복귀 회전 From: %s | To: %s | [정확한 원상복귀]"),
	//	*InitialRotation.Rotator().ToCompactString(),
	//	*TargetRotation.Rotator().ToCompactString());

	Multicast_StartRotation(InitialRotation, TargetRotation, ReturnRotationDuration, true);
}

void ALCTransformGimmick::CompleteRotationReturn()
{
	bIsReturningRotationServer = false;
	bIsRotatingServer = false;

	VisualMesh->SetWorldRotation(OriginalRotation);
	CurrentRotationQuat = OriginalRotationQuat;

	RotationIndex = 0;
	TotalRotationIndex = 0;
}

#pragma endregion

#pragma region Rotation - Interpolation

void ALCTransformGimmick::StartServerRotation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration)
{
	VisualMesh->SetWorldRotation(FromQuat);

    Multicast_StartRotation(FromQuat, ToQuat, Duration, false);

	if (AttachedSyncComponent)
	{
		AttachedSyncComponent->BroadcastStartRotation(FromQuat, ToQuat, Duration);
	}
}

void ALCTransformGimmick::Multicast_StartRotation_Implementation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration, bool bIsReturnRotation)
{
	if (GetNetMode() != NM_DedicatedServer)
	{
		GetWorldTimerManager().ClearTimer(ClientRotationTimer);

		StartClientRotation(FromQuat, ToQuat, Duration, bIsReturnRotation);
	}
}

void ALCTransformGimmick::StartClientRotation(const FQuat& FromQuat, const FQuat& ToQuat, float Duration, bool bReturn)
{
	bIsReturningRotationClient = bReturn;

	ClientStartRotation = FromQuat;

	ClientTargetRotation = ToQuat;
	ClientRotationDuration = Duration;
	ClientRotationElapsed = 0.f;

	// VisualMesh->SetWorldRotation(ClientStartRotation);

	////LOG_Art(Log, TEXT("[클라] StartClientRotation 시작 - 복귀 상태: %s | From: %s | To: %s"),
	//	bIsReturningRotationClient ? TEXT("✔️ 복귀") : TEXT("❌ 일반"),
	//	*ClientStartRotation.Rotator().ToCompactString(),
	//	*ClientTargetRotation.Rotator().ToCompactString()
	//);

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

	if (bIsReturningRotationClient)
	{
		const FQuat NewQuat = FQuat::SlerpFullPath(ClientStartRotation, ClientTargetRotation, Alpha);
		VisualMesh->SetWorldRotation(NewQuat);
	}
	else
	{
		const float StepAngleRad = FMath::Lerp(0.f, FMath::DegreesToRadians(RotationAngle * RotationCount), Alpha);
		const FVector Axis = GetRotationAxisVector(RotationAxisEnum);
		const FQuat DeltaQuat = FQuat(Axis, StepAngleRad);
		const FQuat NewQuat = DeltaQuat * ClientStartRotation;
		VisualMesh->SetWorldRotation(NewQuat);
	}

	if (Alpha >= 1.f || FMath::IsNearlyEqual(Alpha, 1.f, 0.01f))
	{
		GetWorldTimerManager().ClearTimer(ClientRotationTimer);
	}
}

void ALCTransformGimmick::CacheOriginalRotation()
{
	if (!bOriginalRotationCached)
	{
		OriginalRotationQuat = VisualMesh->GetComponentQuat();
		OriginalRotation = VisualMesh->GetComponentRotation();
		bOriginalRotationCached = true;

		//LOG_Art(Log, TEXT(" 최초 회전 상태 저장됨: %s"), *OriginalRotationQuat.Rotator().ToCompactString());
	}
}

FQuat ALCTransformGimmick::GetAlternateRotationQuat() const
{
	const float TotalAngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
	const FVector Axis = GetRotationAxisVector(RotationAxisEnum);
	return FQuat(Axis, TotalAngleRad) * OriginalRotationQuat;
}

#pragma endregion
