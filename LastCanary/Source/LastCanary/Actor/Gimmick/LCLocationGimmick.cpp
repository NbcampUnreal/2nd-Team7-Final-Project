#include "Actor/Gimmick/LCLocationGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCLocationGimmick::ALCLocationGimmick()
	: MoveStep(100.f)
	, MoveVector(FVector::ZeroVector)
	, bUseAxis(false)
	, MoveSpeed(100.f)
	, MoveIndex(0)
	, bIsMovingServer(false)
	, bIsReturningServer(false)
	, ServerMoveElapsed(0.f)
	, ServerMoveDuration(0.f)
	, ClientMoveElapsed(0.f)
	, ClientMoveDuration(0.f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

#pragma region Lifecycle

void ALCLocationGimmick::BeginPlay()
{
	Super::BeginPlay();
	OriginalLocation = GetActorLocation();

	if (bUseAlternateToggle)
	{
		const FVector Delta = bUseAxis ? MoveVector : FVector(0.f, 0.f, MoveStep);
		AlternateLocation = OriginalLocation + Delta;
	}
}

#pragma endregion

#pragma region Activation

void ALCLocationGimmick::ActivateGimmick_Implementation()
{
	if (!HasAuthority())
	{
		Server_ActivateGimmick();
		return;
	}

	if (bIsReturningServer || bIsMovingServer)
	{
		LOG_Art(Log, TEXT("이동 기믹 ▶ 보간 중 상태 초기화 후 이동 시작"));
		GetWorldTimerManager().ClearTimer(MovementTimerHandle);
		GetWorldTimerManager().ClearTimer(ServerMoveTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);
		GetWorldTimerManager().ClearTimer(ClientMoveTimer);

		bIsReturningServer = false;
		bIsMovingServer = false;
		ClientMoveElapsed = 0.f;
		ClientMoveDuration = 0.f;
	}

	if (!ILCGimmickInterface::Execute_CanActivate(this))
	{
		return;
	}

	Super::ActivateGimmick_Implementation();
	
	if (bUseAlternateToggle)
	{
		const FVector CurrentLocation = GetActorLocation();
		const FVector& From = CurrentLocation;
		const FVector& To = CurrentLocation.Equals(OriginalLocation, 1.f) ? AlternateLocation : OriginalLocation;
		const float Duration = FVector::Dist(From, To) / MoveSpeed;

		StartServerMovement(From, To, Duration);

		GetWorldTimerManager().SetTimer(
			MovementTimerHandle,
			this,
			&ALCLocationGimmick::CompleteMovement,
			Duration,
			false
		);

		Multicast_StartMovement(From, To, Duration);
	}
	else
	{
		StartMovement();  
	}
}

bool ALCLocationGimmick::CanActivate_Implementation()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		LOG_Art_WARNING(TEXT("이동 기믹 ▶ 현재 이동 중이라 CanActivate 거부됨"));
		return false;
	}

	return Super::CanActivate_Implementation();
}

void ALCLocationGimmick::ReturnToInitialState_Implementation()
{
	if (bIsReturningServer)
	{
		LOG_Art(Log, TEXT("▶ 이미 복귀 중 - ReturnToInitialState 무시"));
		return;
	}

	// 이동 중이면 멈춤
	if (bIsMovingServer)
	{
		GetWorldTimerManager().ClearTimer(MovementTimerHandle);
		GetWorldTimerManager().ClearTimer(ServerMoveTimer);
		GetWorldTimerManager().ClearTimer(ClientMoveTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

		bIsMovingServer = false;
	}

	// 복귀용 위치 계산
	const FVector From = GetActorLocation();
	const FVector To = OriginalLocation;

	if (From.Equals(To, 1.0f))
	{
		LOG_Art(Log, TEXT("▶ 복귀 위치와 현재 위치 동일 - 복귀 생략"));
		return;
	}

	const float Duration = FVector::Dist(From, To) / MoveSpeed;

	bIsReturningServer = true;

	StartServerMovement(From, To, Duration);

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&ALCLocationGimmick::CompleteReturn,
		Duration,
		false
	);

	Multicast_StartMovement(From, To, Duration);
}

#pragma endregion

#pragma region Movement - Server

void ALCLocationGimmick::StartMovement()
{
	if (bIsMovingServer || bIsReturningServer)
	{
		return;
	}

	InitialLocation = GetActorLocation();
	MoveIndex++;

	const FVector Delta = bUseAxis ? MoveVector : FVector(0.f, 0.f, MoveStep);
	TargetLocation = InitialLocation + Delta;

	const float Distance = Delta.Size();
	const float Duration = Distance / MoveSpeed;

	bIsMovingServer = true;

	GetWorldTimerManager().SetTimer(
		MovementTimerHandle,
		this,
		&ALCLocationGimmick::CompleteMovement,
		Duration,
		false
	);

	Multicast_StartMovement(InitialLocation, TargetLocation, Duration);

	if (!IsRunningDedicatedServer())
	{
		StartClientMovement(InitialLocation, TargetLocation, Duration);
	}
}

void ALCLocationGimmick::CompleteMovement()
{
	bIsMovingServer = false;
	SetActorLocation(TargetLocation);

	if (!bToggleState && !bIsReturningServer)
	{
		GetWorldTimerManager().SetTimer(
			ReturnTimerHandle,
			this,
			&ALCLocationGimmick::ReturnToInitialLocation,
			ReturnDelay,
			false
		);
	}
}

void ALCLocationGimmick::ReturnToInitialLocation()
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
		&ALCLocationGimmick::CompleteReturn,
		Duration,
		false
	);

	Multicast_StartMovement(From, To, Duration);
}

void ALCLocationGimmick::CompleteReturn()
{
	bIsReturningServer = false;
	bIsMovingServer = false;

	SetActorLocation(OriginalLocation);
	TargetLocation = OriginalLocation;
	MoveIndex = 0;
}

#pragma endregion

#pragma region Movement - Server Interpolation

void ALCLocationGimmick::StartServerMovement(const FVector& From, const FVector& To, float Duration)
{
	ServerMoveElapsed = 0.f;
	ServerMoveDuration = Duration;
	InitialLocation = From;
	TargetLocation = To;

	GetWorldTimerManager().SetTimer(
		ServerMoveTimer,
		this,
		&ALCLocationGimmick::StepServerMovement,
		0.02f,
		true
	);
}

void ALCLocationGimmick::StepServerMovement()
{
	ServerMoveElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ServerMoveElapsed / ServerMoveDuration, 0.f, 1.f);
	const FVector NewLoc = FMath::Lerp(InitialLocation, TargetLocation, Alpha);
	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerMoveTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

		SetActorLocation(TargetLocation);

		bIsReturningServer = false;
		bIsMovingServer = false;

		if (TargetLocation.Equals(OriginalLocation, 1.f))
		{
			MoveIndex = 0;
		}
	}
}

void ALCLocationGimmick::StartServerAttachedMovement(const FVector& DeltaLocation, float Duration)
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
				const FVector NewLoc = FMath::Lerp(StartLoc, EndLoc, Alpha);
				Target->SetActorLocation(NewLoc);
			});

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);

		AttachedMovementTimers.Add(Target, Handle);
	}
}

#pragma endregion

#pragma region Movement - Client Interpolation

void ALCLocationGimmick::Multicast_StartMovement_Implementation(const FVector& From, const FVector& To, float Duration)
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

void ALCLocationGimmick::StartClientMovement(const FVector& From, const FVector& To, float Duration)
{
	ensure(!IsRunningDedicatedServer());

	ClientStartLocation = From;
	ClientTargetLocation = To;
	ClientMoveElapsed = 0.f;
	ClientMoveDuration = Duration;

	GetWorldTimerManager().SetTimer(
		ClientMoveTimer,
		this,
		&ALCLocationGimmick::StepClientMovement,
		0.02f,
		true
	);
}

void ALCLocationGimmick::StepClientMovement()
{
	ClientMoveElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ClientMoveElapsed / ClientMoveDuration, 0.f, 1.f);
	const FVector NewLoc = FMath::Lerp(ClientStartLocation, ClientTargetLocation, Alpha);

	SetActorLocation(NewLoc);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ClientMoveTimer);
		SetActorLocation(ClientTargetLocation); 
	}
}

void ALCLocationGimmick::StartClientAttachedMovement(const FVector& DeltaLocation, float Duration)
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
				const FVector NewLoc = FMath::Lerp(StartLoc, EndLoc, Alpha);
				Target->SetActorLocation(NewLoc);
			});

		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, Delegate, 0.02f, true);

		AttachedMovementTimers.Add(Target, Handle);
	}
}

#pragma endregion

