#include "Character/Component/CameraRecoilComponent.h"
#include "Character/BaseCharacter.h"

void UCameraRecoilComponent::BeginPlay()
{
	if (GetCharacter())
	{
		CachedController = Cast<APlayerController>(GetCharacter()->GetController());
	}
}

void UCameraRecoilComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{

}

// 스무스하게 반동주기
void UCameraRecoilComponent::ApplySmoothRecoil(float Vertical, float Horizontal)
{
	if (!GetPlayerController()) return;

	// 목표 반동량 설정
	float ShotMultiplier = FMath::Min(1.0f + (CurrentShotCount * 0.15f), 2.5f);
	TargetRecoil.X += Vertical * ShotMultiplier;
	TargetRecoil.Y += FMath::RandRange(-Horizontal, Horizontal) * ShotMultiplier;

	CurrentShotCount++;

	// 스무딩된 반동 적용 시작
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilRecoveryTimer))
	{
		GetWorld()->GetTimerManager().SetTimer(RecoilRecoveryTimer, this,
			&UCameraRecoilComponent::ApplySmoothRecoilStep, 0.016f, true);
	}

	// 연사 리셋 타이머
	GetWorld()->GetTimerManager().ClearTimer(ShotResetTimer);
	GetWorld()->GetTimerManager().SetTimer(ShotResetTimer, this,
		&UCameraRecoilComponent::ResetShotCounter, 0.25f, false);
}

void UCameraRecoilComponent::ApplySmoothRecoilStep()
{
	if (!GetPlayerController())
	{
		GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		return;
	}

	float DeltaTime = GetWorld()->GetDeltaSeconds();

	// 목표 반동량으로 보간
	FVector2D RecoilDelta = (TargetRecoil - AccumulatedRecoil) * (5.0f * DeltaTime);

	if (!RecoilDelta.IsNearlyZero(0.01f))
	{
		GetCharacter()->AddControllerPitchInput(RecoilDelta.X);
		GetCharacter()->AddControllerYawInput(RecoilDelta.Y);
		AccumulatedRecoil += RecoilDelta;
	}

	// 자동 복구 (사격이 멈춘 후)
	if (GetWorld()->GetTimerManager().GetTimerRemaining(ShotResetTimer) <= 0.0f)
	{
		FVector2D RecoveryDelta = AccumulatedRecoil * (RecoilRecoverySpeed * DeltaTime);

		if (RecoveryDelta.Size() >= AccumulatedRecoil.Size())
		{
			// 완전 복구
			GetCharacter()->AddControllerPitchInput(-AccumulatedRecoil.X * RecoilRecoveryAmount);
			GetCharacter()->AddControllerYawInput(-AccumulatedRecoil.Y * RecoilRecoveryAmount);
			AccumulatedRecoil = FVector2D::ZeroVector;
			TargetRecoil = FVector2D::ZeroVector;
			GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
		}
		else
		{
			GetCharacter()->AddControllerPitchInput(-RecoveryDelta.X * RecoilRecoveryAmount);
			GetCharacter()->AddControllerYawInput(-RecoveryDelta.Y * RecoilRecoveryAmount);
			AccumulatedRecoil -= RecoveryDelta * (1 / RecoilRecoveryAmount);
			TargetRecoil -= RecoveryDelta * (1 / RecoilRecoveryAmount);
		}
	}
}

void UCameraRecoilComponent::ResetShotCounter()
{
	CurrentShotCount = 0;
}

// ===== 이 부분이 핵심! 직접 반동 초기화 함수들 =====

// 반동 완전 리셋
void UCameraRecoilComponent::ResetRecoil()
{
	AccumulatedRecoil = FVector2D::ZeroVector;
	TargetRecoil = FVector2D::ZeroVector;
	GetWorld()->GetTimerManager().ClearTimer(RecoilRecoveryTimer);
}

// 반동 부분 리셋 (특정 축만)
void UCameraRecoilComponent::ResetRecoilPitch()
{
	AccumulatedRecoil.X = 0.0f;
	TargetRecoil.X = 0.0f;
}

void UCameraRecoilComponent::ResetRecoilYaw()
{
	AccumulatedRecoil.Y = 0.0f;
	TargetRecoil.Y = 0.0f;
}

// 반동 감소 (완전 리셋이 아닌 부분 감소)
void UCameraRecoilComponent::ReduceRecoil(float ReductionFactor)
{
	AccumulatedRecoil *= (1.0f - FMath::Clamp(ReductionFactor, 0.0f, 1.0f));
	TargetRecoil *= (1.0f - FMath::Clamp(ReductionFactor, 0.0f, 1.0f));

	// 거의 0에 가까우면 완전 리셋
	if (AccumulatedRecoil.Size() < 0.1f)
	{
		ResetRecoil();
	}
}

// 현재 반동 상태 확인용
FVector2D UCameraRecoilComponent::GetCurrentRecoil() const
{
	return AccumulatedRecoil;
}

bool UCameraRecoilComponent::HasActiveRecoil() const
{
	return !AccumulatedRecoil.IsNearlyZero(0.01f);
}
