#include "Actor/Gimmick/LCAutoGimmick.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "LastCanary.h"

ALCAutoGimmick::ALCAutoGimmick()
	: bStartAtBeginPlay(false)
	, bLoopingEnabled(false)
	, LoopInterval(0.f)
	, LoopType(EGimmickLoopType::None)
	, ForwardMoveDuration(1.0f)
	, BackwardMoveDuration(1.0f)
	, ForwardRotationDuration(1.0f)
	, BackwardRotationDuration(1.0f)
{
	ActivationType = EGimmickActivationType::ActivateOnConditionMet;
	bToggleState = false;
	bUseAlternateToggle = false;
}

void ALCAutoGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bStartAtBeginPlay && LoopType != EGimmickLoopType::None)
	{
		GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::StartLoop, 3.0f, false);
	}
}

void ALCAutoGimmick::StartLoop()
{
	if (!HasAuthority()) return;

	bLoopingEnabled = true;

	const float StartDelay = FMath::FRandRange(0.f, LoopInterval);

	if (LoopInterval > 0.f)
	{
		GetWorldTimerManager().SetTimer(
			LoopTimerHandle,
			this,
			&ALCAutoGimmick::HandleLoop,
			LoopInterval,
			true, 
			StartDelay 
		);
	}
	else
	{
		HandleLoop();
	}
}

void ALCAutoGimmick::StopLoop()
{
	if (!HasAuthority()) return;

	bLoopingEnabled = false;
	GetWorldTimerManager().ClearTimer(LoopTimerHandle);
}

void ALCAutoGimmick::HandleLoop()
{
	if (!HasAuthority() || !bLoopingEnabled || LoopType == EGimmickLoopType::None)
		return;

	//LOG_Art(Log, TEXT("[HandleLoop] 루프 실행 - 타입: %d"), static_cast<int32>(LoopType));

	if (IsGimmickBusy_Implementation())
	{
		GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::HandleLoop, 0.1f, false);
		return;
	}

	switch (LoopType)
	{
	case EGimmickLoopType::LoopForward:
	{
		LOG_Art(Log, TEXT("[HandleLoop] LoopForward - 회전 상태 초기화 및 실행"));

		bUseAlternateToggle = false;

		CurrentRotationQuat = VisualMesh->GetComponentQuat();
		OriginalRotationQuat = CurrentRotationQuat;
		InitialRotation = CurrentRotationQuat;

		StartMovement();
		StartRotation();
		break;
	}

	case EGimmickLoopType::PingPong:
	{
		//LOG_Art(Log, TEXT("[HandleLoop] PingPong - ActivateGimmick 호출"));

		bUseAlternateToggle = true;

		if (OriginalRotationQuat.Equals(FQuat::Identity, 0.01f))
		{
			OriginalRotationQuat = VisualMesh->GetComponentQuat();
			CurrentRotationQuat = OriginalRotationQuat;
			RotationIndex = 0; 

			//LOG_Art(Log, TEXT("✅ 최초 회전 상태 저장됨: %s"), *OriginalRotationQuat.Rotator().ToCompactString());
		}
		
		ILCGimmickInterface::Execute_ActivateGimmick(this);
		break;
	}



	default:
		break;
	}
}

void ALCAutoGimmick::ScheduleNextLoop()
{
	//LOG_Art(Log, TEXT("🔄 [ScheduleNextLoop] 호출됨 - Interval: %.2f"), LoopInterval);

	if (!HasAuthority() || !bLoopingEnabled) return;

	if (LoopInterval <= 0.f)
	{
		//LOG_Art(Log, TEXT("▶ [ScheduleNextLoop] 즉시 HandleLoop 호출"));
		HandleLoop();
	}
	else
	{
		//LOG_Art(Log, TEXT("▶ [ScheduleNextLoop] 타이머로 HandleLoop 예약"));
		GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::HandleLoop, LoopInterval, false);
	}
}

void ALCAutoGimmick::ActivateGimmick_Implementation()
{
	if (!HasAuthority()) return;
	if (!ILCGimmickInterface::Execute_CanActivate(this)) return;

	const bool bIsForward = (RotationIndex % 2 == 0);

	//LOG_Art(Log, TEXT("▶ [ActivateGimmick] 실행"));

	if (bIsForward && ForwardSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ForwardSound, GetActorLocation());
	}
	else if (!bIsForward && BackwardSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, BackwardSound, GetActorLocation());
	}

	if (LoopType == EGimmickLoopType::PingPong)
	{
		if (RotationIndex == 0)
		{
			CacheOriginalRotation();
		}

		//LOG_Art(Log, TEXT("[ActivateGimmick] PingPong - 방향: %s"), bIsForward ? TEXT("Forward") : TEXT("Backward"));

		MoveDuration = bIsForward ? ForwardMoveDuration : BackwardMoveDuration;
		RotationDuration = bIsForward ? ForwardRotationDuration : BackwardRotationDuration;

		if (!MoveVector.IsNearlyZero())
		{
			const FVector BaseMoveVector = MoveVector.GetSafeNormal() * MoveVector.Size();
			MoveVector = bIsForward ? BaseMoveVector : -BaseMoveVector;
		}

		if (bIsForward)
		{
			InitialRotation = OriginalRotationQuat;
			const FVector Axis = GetRotationAxisVector(RotationAxisEnum);
			const float AngleRad = FMath::DegreesToRadians(RotationAngle * RotationCount);
			const FQuat DeltaQuat = FQuat(Axis, AngleRad);
			TargetRotation = DeltaQuat * InitialRotation;
			CurrentRotationQuat = TargetRotation;
		}
		else
		{
			InitialRotation = CurrentRotationQuat;
			TargetRotation = OriginalRotationQuat;
			CurrentRotationQuat = TargetRotation;
		}

		RotationIndex++;
	}

	Super::ActivateGimmick_Implementation();
}
void ALCAutoGimmick::DeactivateGimmick_Implementation()
{
	if (!HasAuthority()) return;

	StopLoop();
	Super::DeactivateGimmick_Implementation();
}

void ALCAutoGimmick::CompleteMovement()
{
	//LOG_Art(Log, TEXT("✔️ [CompleteMovement] 이동 완료"));

	const bool bShouldReturn = !bLoopingEnabled && !bToggleState;

	if (bShouldReturn)
	{
		//LOG_Art(Log, TEXT("▶ [CompleteMovement] 복귀 예약"));
		Super::CompleteMovement();
	}
	else
	{
		SetActorLocation(TargetLocation);
		bIsMovingServer = false;
	}

	// ✅ 루프 처리
	if (bLoopingEnabled)
	{
		if (LoopInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::HandleLoop, LoopInterval, false);
		}
		else
		{
			HandleLoop(); // 즉시 반복
		}
	}
}

void ALCAutoGimmick::CompleteRotation()
{
	//LOG_Art(Log, TEXT("✔️ [CompleteRotation] 회전 완료"));

	const bool bShouldReturn = !bLoopingEnabled && !bToggleState;

	if (bShouldReturn)
	{
		LOG_Art(Log, TEXT("▶ [CompleteRotation] 복귀 예약"));
		Super::CompleteRotation();
	}
	else
	{
		VisualMesh->SetWorldRotation(TargetRotation);
		bIsRotatingServer = false;
	}

	// ✅ 루프 처리
	if (bLoopingEnabled)
	{
		if (LoopInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::HandleLoop, LoopInterval, false);
		}
		else
		{
			HandleLoop(); // 즉시 반복
		}
	}
}

void ALCAutoGimmick::CompleteReturn()
{
	//LOG_Art(Log, TEXT("✅ [CompleteReturn] 복귀 이동 완료"));
	Super::CompleteReturn();

	if (LoopType == EGimmickLoopType::PingPong && bLoopingEnabled)
	{
		if (LoopInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::HandleLoop, LoopInterval, false);
		}
		else
		{
			HandleLoop();
		}
	}
}

void ALCAutoGimmick::CompleteRotationReturn()
{
	//LOG_Art(Log, TEXT("✅ [CompleteRotationReturn] 복귀 회전 완료"));
	Super::CompleteRotationReturn();

	if (LoopType == EGimmickLoopType::PingPong && bLoopingEnabled)
	{
		if (LoopInterval > 0.f)
		{
			GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::HandleLoop, LoopInterval, false);
		}
		else
		{
			HandleLoop();
		}
	}
}

void ALCAutoGimmick::ReturnToInitialState_Implementation()
{
	//LOG_Art(Log, TEXT("🔁 [ReturnToInitialState] 복귀 시작"));

	if (LoopType == EGimmickLoopType::PingPong)
	{
		ReturnMoveDuration = BackwardMoveDuration;
		ReturnRotationDuration = BackwardRotationDuration;
	}

	Super::ReturnToInitialState_Implementation();

	if (!HasAuthority()) return;

	StopLoop();

	if (LoopRestartDelay > 0.f && LoopType != EGimmickLoopType::None)
	{
		//LOG_Art(Log, TEXT("⏳ [ReturnToInitialState] %.2f초 후 루프 재시작 예약"), LoopRestartDelay);
		GetWorldTimerManager().SetTimer(LoopTimerHandle, this, &ALCAutoGimmick::StartLoop, LoopRestartDelay, false);
	}
}
