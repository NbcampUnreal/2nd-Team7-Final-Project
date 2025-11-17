#include "Actor/Gimmick/LCTrackingGimmick.h"
#include "Components/PointLightComponent.h"
#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/KismetMathLibrary.h"
#include "TimerManager.h"
#include "LastCanary.h"

ALCTrackingGimmick::ALCTrackingGimmick()
	: TargetActor(nullptr)
	, TrackingInterval(0.2f)
	, YawOffset(0.f)
	, EffectYawOffset(0.f)
	, SpotLightDuration(2.f)
	, bIsTracking(false)
{
	PrimaryActorTick.bCanEverTick = false;

	TrackingPointLightLeft = CreateDefaultSubobject<UPointLightComponent>(TEXT("TrackingPointLightLeft"));
	TrackingPointLightLeft->SetupAttachment(VisualMesh);
	TrackingPointLightLeft->SetIntensity(20000.f);
	TrackingPointLightLeft->SetAttenuationRadius(3000.f);
	TrackingPointLightLeft->SetVisibility(false);
	TrackingPointLightLeft->SetIsReplicated(true);

	TrackingPointLightRight = CreateDefaultSubobject<UPointLightComponent>(TEXT("TrackingPointLightRight"));
	TrackingPointLightRight->SetupAttachment(VisualMesh);
	TrackingPointLightRight->SetIntensity(20000.f);
	TrackingPointLightRight->SetAttenuationRadius(3000.f);
	TrackingPointLightRight->SetVisibility(false);
	TrackingPointLightRight->SetIsReplicated(true);

	TrackingSpotLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("TrackingSpotLight"));
	TrackingSpotLight->SetupAttachment(RootComponent);
	TrackingSpotLight->SetIntensity(50000.f);
	TrackingSpotLight->SetAttenuationRadius(6000.f);
	TrackingSpotLight->SetOuterConeAngle(25.f);
	TrackingSpotLight->SetInnerConeAngle(5.f);
	TrackingSpotLight->SetVisibility(false);
	TrackingSpotLight->SetIsReplicated(true);

	bReplicates = true;
}

void ALCTrackingGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (TrackingPointLightLeft) TrackingPointLightLeft->SetVisibility(false);
	if (TrackingPointLightRight) TrackingPointLightRight->SetVisibility(false);
	if (TrackingSpotLight) TrackingSpotLight->SetVisibility(false);
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

	const float DelayOffset = FMath::FRandRange(0.f, TrackingInterval);

	GetWorldTimerManager().SetTimer(
		TrackingTimerHandle,
		this,
		&ALCTrackingGimmick::RotateToTarget,
		TrackingInterval,
		true,
		DelayOffset
	);

	if (TrackingPointLightLeft)
	{
		TrackingPointLightLeft->SetLightColor(FLinearColor::White);
		TrackingPointLightLeft->SetVisibility(true);
	}
	if (TrackingPointLightRight)
	{
		TrackingPointLightRight->SetLightColor(FLinearColor::White);
		TrackingPointLightRight->SetVisibility(true);
	}
}

void ALCTrackingGimmick::StopTracking()
{
	if (!HasAuthority()) return;

	bIsTracking = false;
	GetWorldTimerManager().ClearTimer(TrackingTimerHandle);
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	if (TrackingPointLightLeft)  TrackingPointLightLeft->SetVisibility(false);
	if (TrackingPointLightRight) TrackingPointLightRight->SetVisibility(false);
	if (TrackingSpotLight)       TrackingSpotLight->SetVisibility(false);
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

	SetActorRotation(FRotator(0.f, DesiredYaw, 0.f));

	// 라이트 위치 → 타겟 위
	const float SpotLightHeightOffset = 500.f;
	FVector TargetLoc = TargetActor->GetActorLocation();
	FVector SpotLoc(TargetLoc.X, TargetLoc.Y, TargetLoc.Z + SpotLightHeightOffset);

	if (TrackingSpotLight)
	{
		TrackingSpotLight->SetWorldLocation(SpotLoc);

		// 타겟 바라보는 회전값으로 라이트 회전
		FRotator LookAtRot = UKismetMathLibrary::FindLookAtRotation(SpotLoc, TargetLoc);
		TrackingSpotLight->SetWorldRotation(LookAtRot);
	}
}

void ALCTrackingGimmick::Fire()
{
	if (!HasAuthority() || !IsValid(TargetActor)) return;

	// 클라이언트에 시각적 효과 전파
	Multicast_FireEffect();
}

void ALCTrackingGimmick::Multicast_FireEffect_Implementation()
{
	// 포인트 라이트 → 빨간색으로 변경
	if (TrackingPointLightLeft)
	{
		TrackingPointLightLeft->SetVisibility(true);
		TrackingPointLightLeft->SetLightColor(FLinearColor::Red);
	}

	if (TrackingPointLightRight)
	{
		TrackingPointLightRight->SetVisibility(true);
		TrackingPointLightRight->SetLightColor(FLinearColor::Red);
	}

	// SpotLight 켜기
	if (TrackingSpotLight)
	{
		TrackingSpotLight->SetVisibility(true);

		GetWorldTimerManager().SetTimer(
			FireTimerHandle,
			[this]()
			{
				if (TrackingSpotLight)
				{
					TrackingSpotLight->SetVisibility(false);
				}

				if (TrackingPointLightLeft)
				{
					TrackingPointLightLeft->SetVisibility(true);
					TrackingPointLightLeft->SetLightColor(FLinearColor::White);
				}

				if (TrackingPointLightRight)
				{
					TrackingPointLightRight->SetVisibility(true);
					TrackingPointLightRight->SetLightColor(FLinearColor::White);
				}
			},
			SpotLightDuration,
			false
		);
	}
}

void ALCTrackingGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCTrackingGimmick, TargetActor);
}
