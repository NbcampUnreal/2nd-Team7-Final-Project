#include "Actor/Gimmick/LCTrackingGimmick.h"
#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "LastCanary.h"

ALCTrackingGimmick::ALCTrackingGimmick()
	: TargetActor(nullptr)
	, TrackingInterval(0.2f)
	, YawOffset(0.f)
	, bIsTracking(false)
{
	PrimaryActorTick.bCanEverTick = false;

	TrackingLightLeft = CreateDefaultSubobject<USpotLightComponent>(TEXT("TrackingLightLeft"));
	TrackingLightLeft->SetupAttachment(VisualMesh);
	TrackingLightLeft->SetVisibility(false);
	TrackingLightLeft->SetIntensity(200000.f); // 30,000 거리 기준 고출력
	TrackingLightLeft->SetAttenuationRadius(35000.f);
	TrackingLightLeft->SetOuterConeAngle(20.f);
	TrackingLightLeft->SetInnerConeAngle(5.f);
	TrackingLightLeft->SetIsReplicated(true);

	TrackingLightRight = CreateDefaultSubobject<USpotLightComponent>(TEXT("TrackingLightRight"));
	TrackingLightRight->SetupAttachment(VisualMesh);
	TrackingLightRight->SetVisibility(false);
	TrackingLightRight->SetIntensity(200000.f);
	TrackingLightRight->SetAttenuationRadius(35000.f);
	TrackingLightRight->SetOuterConeAngle(20.f);
	TrackingLightRight->SetInnerConeAngle(5.f);
	TrackingLightRight->SetIsReplicated(true);

	bReplicates = true;
}

void ALCTrackingGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (TrackingLightLeft) TrackingLightLeft->SetVisibility(false);
	if (TrackingLightRight) TrackingLightRight->SetVisibility(false);
}


void ALCTrackingGimmick::SetTargetActor(AActor* NewTarget)
{
	if (!HasAuthority()) return;

	TargetActor = NewTarget;

	if (IsValid(TargetActor))
	{
		StartTracking();
	}
	else
	{
		StopTracking();
	}
}

AActor* ALCTrackingGimmick::GetTargetActor() const
{
	return TargetActor;
}

void ALCTrackingGimmick::StartTracking()
{
	if (!HasAuthority() || bIsTracking || !IsValid(TargetActor)) return;

	bIsTracking = true;

	// ▶ 타워 간 타이머 분산을 위한 딜레이
	const float DelayOffset = FMath::FRandRange(0.f, TrackingInterval);

	GetWorldTimerManager().SetTimer(
		TrackingTimerHandle,
		this,
		&ALCTrackingGimmick::RotateToTarget,
		TrackingInterval,
		true,
		DelayOffset
	);
}

void ALCTrackingGimmick::StopTracking()
{
	if (!HasAuthority()) return;

	bIsTracking = false;
	GetWorldTimerManager().ClearTimer(TrackingTimerHandle);
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	if (TrackingLightLeft)  TrackingLightLeft->SetVisibility(false);
	if (TrackingLightRight) TrackingLightRight->SetVisibility(false);
}

void ALCTrackingGimmick::RotateToTarget()
{
	if (!IsValid(TargetActor))
	{
		StopTracking();
		return;
	}

	const FVector From = GetActorLocation();
	const FVector To = TargetActor->GetActorLocation();
	const float DesiredYaw = UKismetMathLibrary::FindLookAtRotation(From, To).Yaw + YawOffset;

	if (!FMath::IsNearlyEqual(GetActorRotation().Yaw, DesiredYaw, 1.f))
	{
		SetActorRotation(FRotator(0.f, DesiredYaw, 0.f));
	}

	const FVector TargetLoc = TargetActor->GetActorLocation();

	if (TrackingLightLeft)
	{
		const FVector LightLoc = TrackingLightLeft->GetComponentLocation();
		const float NewPitch = UKismetMathLibrary::FindLookAtRotation(LightLoc, TargetLoc).Pitch;
		const float OldPitch = TrackingLightLeft->GetRelativeRotation().Pitch;

		if (!FMath::IsNearlyEqual(NewPitch, OldPitch, 1.f))
		{
			TrackingLightLeft->SetRelativeRotation(FRotator(NewPitch, EffectYawOffset, 0.f));
		}
	}

	if (TrackingLightRight)
	{
		const FVector LightLoc = TrackingLightRight->GetComponentLocation();
		const float NewPitch = UKismetMathLibrary::FindLookAtRotation(LightLoc, TargetLoc).Pitch;
		const float OldPitch = TrackingLightRight->GetRelativeRotation().Pitch;

		if (!FMath::IsNearlyEqual(NewPitch, OldPitch, 1.f))
		{
			TrackingLightRight->SetRelativeRotation(FRotator(NewPitch, EffectYawOffset, 0.f));
		}
	}
}

void ALCTrackingGimmick::Fire()
{
	if (!HasAuthority() || !IsValid(TargetActor)) return;

	Multicast_FireEffect();

	if (TrackingLightLeft)  TrackingLightLeft->SetVisibility(true);
	if (TrackingLightRight) TrackingLightRight->SetVisibility(true);
}

void ALCTrackingGimmick::Multicast_FireEffect_Implementation()
{
	if (TrackingLightLeft)  TrackingLightLeft->SetVisibility(true);
	if (TrackingLightRight) TrackingLightRight->SetVisibility(true);
}

void ALCTrackingGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCTrackingGimmick, TargetActor);
}
