#include "Actor/Gimmick/LCRotationGimmick.h"
#include "TimerManager.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ALCRotationGimmick::ALCRotationGimmick()
	: RotationStep(45.f)
	, RotationAxis(FVector::ZeroVector)
	, bUseAxis(false)
	, RotationSpeed(90.f)
	, RotationIndex(0)
	, bIsRotatingServer(false)
	, bIsReturningServer(false)
	, ClientStartRotation(FRotator::ZeroRotator)
	, ClientTargetRotation(FRotator::ZeroRotator)
	, ClientRotationDuration(0.f)
	, ClientRotationElapsed(0.f)
	, ServerRotationDuration(0.f)
	, ServerRotationElapsed(0.f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
}

#pragma region Lifecycle

void ALCRotationGimmick::BeginPlay()
{
	Super::BeginPlay();
	OriginalRotation = VisualMesh->GetComponentRotation();

	if (bUseAlternateToggle)
	{
		const FRotator DeltaRot = bUseAxis
			? FRotator(RotationAxis.X, RotationAxis.Y, RotationAxis.Z)
			: FRotator(0.f, RotationStep, 0.f);

		AlternateRotation = OriginalRotation + DeltaRot;
	}
}

#pragma endregion

#pragma region Activation

void ALCRotationGimmick::ActivateGimmick_Implementation()
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

	if (bUseAlternateToggle)
	{
		const FRotator CurrentRot = VisualMesh->GetComponentRotation();
		const FRotator& From = CurrentRot;
		const FRotator& To = CurrentRot.Equals(OriginalRotation, 1.f) ? AlternateRotation : OriginalRotation;
		const float Duration = (FMath::Abs(From.Pitch - To.Pitch) + FMath::Abs(From.Yaw - To.Yaw) + FMath::Abs(From.Roll - To.Roll)) / RotationSpeed;

		StartServerRotation(From, To, Duration);

		GetWorldTimerManager().SetTimer(
			RotationTimerHandle,
			this,
			&ALCRotationGimmick::CompleteRotation,
			Duration,
			false
		);

		Multicast_StartRotation(From, To, Duration);
	}
	else
	{
		StartRotation();
	}
}

bool ALCRotationGimmick::CanActivate_Implementation()
{
	if (bIsRotatingServer || bIsReturningServer)
	{
		LOG_Art_WARNING(TEXT("회전 기믹 ▶ 현재 회전 중이라 CanActivate 거부됨"));
		return false;
	}

	return Super::CanActivate_Implementation();
}

bool ALCRotationGimmick::IsGimmickBusy_Implementation()
{
	return bIsRotatingServer || bIsReturningServer;
}

void ALCRotationGimmick::ReturnToInitialState_Implementation()
{
	if (bIsReturningServer)
	{
		LOG_Art(Log, TEXT("▶ 이미 복귀 중 - ReturnToInitialState 무시"));
		return;
	}

	// 회전 중이면 멈춤
	if (bIsRotatingServer)
	{
		GetWorldTimerManager().ClearTimer(RotationTimerHandle);
		GetWorldTimerManager().ClearTimer(ServerRotationTimer);
		GetWorldTimerManager().ClearTimer(ClientRotationTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

		bIsRotatingServer = false;
	}

	const FRotator From = VisualMesh->GetComponentRotation();
	const FRotator To = OriginalRotation;

	if (From.Equals(To, 1.0f))
	{
		LOG_Art(Log, TEXT("▶ 복귀 회전값과 현재 동일 - 복귀 생략"));
		return;
	}

	const float Duration = (FMath::Abs(From.Pitch - To.Pitch) + FMath::Abs(From.Yaw - To.Yaw) + FMath::Abs(From.Roll - To.Roll)) / RotationSpeed;

	bIsReturningServer = true;

	StartServerRotation(From, To, Duration);

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCRotationGimmick::CompleteReturn,
		Duration,
		false
	);

	Multicast_StartRotation(From, To, Duration);
}

#pragma endregion

#pragma region Rotation - Server

void ALCRotationGimmick::StartRotation()
{
	if (bIsRotatingServer || bIsReturningServer)
	{
		return;
	}

	InitialRotation = VisualMesh->GetComponentRotation();
	RotationIndex++;

	const FRotator DeltaRot = bUseAxis
		? FRotator(RotationAxis.X, RotationAxis.Y, RotationAxis.Z)
		: FRotator(0.f, RotationStep, 0.f);

	TargetRotation = InitialRotation + DeltaRot;
	const float Duration = (FMath::Abs(DeltaRot.Pitch) + FMath::Abs(DeltaRot.Yaw) + FMath::Abs(DeltaRot.Roll)) / RotationSpeed;

	bIsRotatingServer = true;

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCRotationGimmick::CompleteRotation,
		Duration,
		false
	);

	Multicast_StartRotation(InitialRotation, TargetRotation, Duration);

	if (!IsRunningDedicatedServer())
	{
		StartClientRotation(InitialRotation, TargetRotation, Duration);
	}
}

void ALCRotationGimmick::CompleteRotation()
{
	bIsRotatingServer = false;
	VisualMesh->SetWorldRotation(TargetRotation);

	AttachedActors.RemoveAll([this](AActor* Actor)
		{
			return !DetectionArea->IsOverlappingActor(Actor);
		});

	if (!bToggleState)
	{
		GetWorldTimerManager().SetTimer(
			ReturnTimerHandle,
			this,
			&ALCRotationGimmick::ReturnToInitialRotation,
			ReturnDelay,
			false
		);
	}
}

void ALCRotationGimmick::ReturnToInitialRotation()
{
	if (bIsRotatingServer || bIsReturningServer)
	{
		return;
	}

	bIsReturningServer = true;

	const FRotator From = VisualMesh->GetComponentRotation();
	const FRotator To = OriginalRotation;
	const FRotator DeltaRot = To - From;
	const float Duration = (FMath::Abs(DeltaRot.Pitch) + FMath::Abs(DeltaRot.Yaw) + FMath::Abs(DeltaRot.Roll)) / RotationSpeed;

	StartServerRotation(From, To, Duration);

	GetWorldTimerManager().SetTimer(
		RotationTimerHandle,
		this,
		&ALCRotationGimmick::CompleteReturn,
		Duration,
		false
	);

	Multicast_StartRotation(From, To, Duration);
}

void ALCRotationGimmick::CompleteReturn()
{
	bIsReturningServer = false;

	VisualMesh->SetWorldRotation(OriginalRotation);
	TargetRotation = OriginalRotation;
	RotationIndex = 0;

	AttachedActors.RemoveAll([this](AActor* Actor)
		{
			return !DetectionArea->IsOverlappingActor(Actor);
		});
}


#pragma endregion

#pragma region Rotation - Server Interpolation

void ALCRotationGimmick::StartServerRotation(const FRotator& From, const FRotator& To, float Duration)
{
	ServerRotationElapsed = 0.f;
	ServerRotationDuration = Duration;
	InitialRotation = From;
	TargetRotation = To;

	GetWorldTimerManager().SetTimer(
		ServerRotationTimer,
		this,
		&ALCRotationGimmick::StepServerRotation,
		0.02f,
		true
	);
}

void ALCRotationGimmick::StepServerRotation()
{
	ServerRotationElapsed += 0.02f;
	const float Alpha = FMath::Clamp(ServerRotationElapsed / ServerRotationDuration, 0.f, 1.f);
	const FRotator NewRot = FMath::Lerp(InitialRotation, TargetRotation, Alpha);

	VisualMesh->SetWorldRotation(NewRot);

	if (Alpha >= 1.f)
	{
		GetWorldTimerManager().ClearTimer(ServerRotationTimer);
		GetWorldTimerManager().ClearTimer(ReturnTimerHandle);

		bIsReturningServer = false;
		VisualMesh->SetWorldRotation(TargetRotation);
	}
}

void ALCRotationGimmick::StartServerAttachedRotation(const FRotator& DeltaRot, float Duration)
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

		FTimerDelegate RotateDelegate;
		RotateDelegate.BindLambda([=]()
			{
				if (!IsValid(Target)) return;

				*ElapsedTime += 0.02f;
				const float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.f, 1.f);
				const FRotator NewRot = FMath::Lerp(StartRot, EndRot, Alpha);
				Target->SetActorRotation(NewRot);
			});

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, RotateDelegate, 0.02f, true);

		AttachedRotationTimers.Add(Target, TimerHandle);
	}
}

#pragma endregion

#pragma region Rotation - Client Interpolation

void ALCRotationGimmick::Multicast_StartRotation_Implementation(const FRotator& From, const FRotator& To, float Duration)
{
	const FRotator DeltaRot = To - From;

	if (!HasAuthority())
	{
		StartClientRotation(From, To, Duration);
		StartClientAttachedRotation(DeltaRot, Duration);
	}
	else
	{
		StartServerAttachedRotation(DeltaRot, Duration); 
	}
}

void ALCRotationGimmick::StartClientRotation(const FRotator& From, const FRotator& To, float Duration)
{
	ensure(!IsRunningDedicatedServer());

	ClientStartRotation = From;
	ClientTargetRotation = To;
	ClientRotationDuration = Duration;
	ClientRotationElapsed = 0.f;

	GetWorldTimerManager().SetTimer(
		ClientRotationTimer,
		this,
		&ALCRotationGimmick::StepClientRotation,
		0.02f,
		true
	);
}

void ALCRotationGimmick::StepClientRotation()
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

void ALCRotationGimmick::StartClientAttachedRotation(const FRotator& DeltaRot, float Duration)
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

		FTimerDelegate RotateDelegate;
		RotateDelegate.BindLambda([=]()
			{
				if (!IsValid(Target)) return;

				*ElapsedTime += 0.02f;
				const float Alpha = FMath::Clamp(*ElapsedTime / Duration, 0.f, 1.f);
				const FRotator NewRot = FMath::Lerp(StartRot, EndRot, Alpha);
				Target->SetActorRotation(NewRot);
			});

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, RotateDelegate, 0.02f, true);

		AttachedRotationTimers.Add(Target, TimerHandle);
	}
}

#pragma endregion
