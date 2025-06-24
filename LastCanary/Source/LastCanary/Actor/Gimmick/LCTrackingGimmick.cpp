#include "Actor/Gimmick/LCTrackingGimmick.h"
#include "NiagaraComponent.h"
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

	TrackingEffectLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrackingEffectLeft"));
	TrackingEffectLeft->SetupAttachment(VisualMesh);
	TrackingEffectLeft->bAutoActivate = false;
	TrackingEffectLeft->SetIsReplicated(true);

	TrackingEffectRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("TrackingEffectRight"));
	TrackingEffectRight->SetupAttachment(VisualMesh);
	TrackingEffectRight->bAutoActivate = false;
	TrackingEffectRight->SetIsReplicated(true);

	bReplicates = true;
}

void ALCTrackingGimmick::BeginPlay()
{
	Super::BeginPlay();

	if (TrackingEffectLeft)
	{
		TrackingEffectLeft->Deactivate();
	}
	if (TrackingEffectRight)
	{
		TrackingEffectRight->Deactivate();
	}

	//LOG_Art(Log, TEXT(" BeginPlay - 감시탑 초기화 완료"));

}
void ALCTrackingGimmick::SetTargetActor(AActor* NewTarget)
{
	if (!HasAuthority()) return;

	TargetActor = NewTarget;

	if (IsValid(TargetActor))
	{
		//LOG_Art(Log, TEXT(" 타겟 설정됨: %s"), *TargetActor->GetName());
		StartTracking();
	}
	else
	{
		//LOG_Art(Log, TEXT(" 타겟 해제됨 → 추적 중단"));
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
	GetWorldTimerManager().SetTimer(TrackingTimerHandle, this, &ALCTrackingGimmick::RotateToTarget, TrackingInterval, true);

	//LOG_Art(Log, TEXT(" 타겟 추적 시작"));
}

void ALCTrackingGimmick::StopTracking()
{
	if (!HasAuthority()) return;

	bIsTracking = false;
	GetWorldTimerManager().ClearTimer(TrackingTimerHandle);
	GetWorldTimerManager().ClearTimer(FireTimerHandle);

	if (TrackingEffectLeft)
	{
		TrackingEffectLeft->Deactivate();
	}
	if (TrackingEffectRight)
	{
		TrackingEffectRight->Deactivate();
	}

	//LOG_Art(Log, TEXT("⏹ 추적 정지 및 이펙트 비활성화"));
}

void ALCTrackingGimmick::RotateToTarget()
{
	if (!IsValid(TargetActor))
	{
		StopTracking();
		return;
	}

	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), TargetActor->GetActorLocation());

	FRotator NewRotation(0.f, LookAtRotation.Yaw + YawOffset, 0.f);
	SetActorRotation(NewRotation);

	if (TrackingEffectLeft)
	{
		FVector LeftLoc = TrackingEffectLeft->GetComponentLocation();
		FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(LeftLoc, TargetActor->GetActorLocation());

		TrackingEffectLeft->SetRelativeRotation(FRotator(LookRot.Pitch, EffectYawOffset, 0.f));
	}

	if (TrackingEffectRight)
	{
		FVector RightLoc = TrackingEffectRight->GetComponentLocation();
		FRotator LookRot = UKismetMathLibrary::FindLookAtRotation(RightLoc, TargetActor->GetActorLocation());

		TrackingEffectRight->SetRelativeRotation(FRotator(LookRot.Pitch, EffectYawOffset, 0.f));
	}


	//LOG_Art(Log, TEXT(" 타겟 방향 회전 + 이펙트 Pitch 조정: %s | Offset: %.1f"), *TargetActor->GetName(), YawOffset);
}

void ALCTrackingGimmick::Fire()
{
	if (!HasAuthority()) return;
	if (!IsValid(TargetActor)) return;

	//LOG_Art(Log, TEXT(" 이펙트 발사 시작 (%s)"), *GetName());

	Multicast_FireEffect();

	if (TrackingEffectLeft)  TrackingEffectLeft->Activate(true);
	if (TrackingEffectRight) TrackingEffectRight->Activate(true);
}


void ALCTrackingGimmick::Multicast_FireEffect_Implementation()
{
	if (TrackingEffectLeft)
	{
		TrackingEffectLeft->Activate(true);
	}
	if (TrackingEffectRight)
	{
		TrackingEffectRight->Activate(true);
	}
}

void ALCTrackingGimmick::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCTrackingGimmick, TargetActor);
}
