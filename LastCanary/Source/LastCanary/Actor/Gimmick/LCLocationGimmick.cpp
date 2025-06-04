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

	if (!ILCGimmickInterface::Execute_CanActivate(this))
	{
		return;
	}

	Super::ActivateGimmick_Implementation();
	StartMovement();
}

bool ALCLocationGimmick::CanActivate_Implementation()
{
	return Super::CanActivate_Implementation();
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

	if (!bToggleState)
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

		bIsReturningServer = false;
		SetActorLocation(TargetLocation);
	}
}

#pragma endregion

#pragma region Movement - Client Interpolation

void ALCLocationGimmick::Multicast_StartMovement_Implementation(const FVector& From, const FVector& To, float Duration)
{
	if (!HasAuthority())
	{
		StartClientMovement(From, To, Duration);
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
	}
}

#pragma endregion

