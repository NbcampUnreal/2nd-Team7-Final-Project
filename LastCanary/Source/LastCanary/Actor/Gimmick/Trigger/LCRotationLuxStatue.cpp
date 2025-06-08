#include "LCRotationLuxStatue.h"
#include "Interface/LCGimmickInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "LastCanary.h"

ALCRotationLuxStatue::ALCRotationLuxStatue()
	: bIsLuxActive(false)
	, LightRange(2000.f)
	, bUseDebugLine(true)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	LightOriginLeft = CreateDefaultSubobject<USceneComponent>(TEXT("LightOriginLeft"));
	LightOriginLeft->SetupAttachment(VisualMesh);
	LightOriginLeft->SetRelativeLocation(FVector(100.f, -30.f, 50.f));

	LightOriginRight = CreateDefaultSubobject<USceneComponent>(TEXT("LightOriginRight"));
	LightOriginRight->SetupAttachment(VisualMesh);
	LightOriginRight->SetRelativeLocation(FVector(100.f, 30.f, 50.f));

	LightEffectComponentLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightEffectLeft"));
	LightEffectComponentLeft->SetupAttachment(LightOriginLeft);
	LightEffectComponentLeft->SetAutoActivate(false);

	LightEffectComponentRight = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightEffectRight"));
	LightEffectComponentRight->SetupAttachment(LightOriginRight);
	LightEffectComponentRight->SetAutoActivate(false);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(RootComponent);
	AudioComponent->SetAutoActivate(false);
}

void ALCRotationLuxStatue::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && bIsLuxActive)
	{
		GetWorldTimerManager().SetTimer(LuxEmitTimer, this, &ALCRotationLuxStatue::EmitLuxRay, 0.2f, true);
	}
}

void ALCRotationLuxStatue::ActivateLux()
{
	if (!HasAuthority() || bIsLuxActive) return;

	bIsLuxActive = true;

	LOG_Art(Log, TEXT("Lux 활성화"));

	if (LightActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LightActivateSound, GetActorLocation());
	}
	Multicast_PlayLightSound();

	GetWorldTimerManager().SetTimer(LuxEmitTimer, this, &ALCRotationLuxStatue::EmitLuxRay, 0.2f, true);
}

bool ALCRotationLuxStatue::IsLuxActive() const
{
	return bIsLuxActive;
}

void ALCRotationLuxStatue::EmitLuxRay()
{
	if (!HasAuthority() || !bIsLuxActive) return;

	const FVector Start = LightOriginLeft->GetComponentLocation();
	const FVector End = Start + LightOriginLeft->GetForwardVector() * LightRange;

	// 원기둥 형태의 스피어 트레이스 사용
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const float TraceRadius = 100.f; // 감지 반경 설정 (원한다면 조절 가능)

	bool bHit = GetWorld()->SweepSingleByChannel(
		Hit,
		Start,
		End,
		FQuat::Identity,
		ECC_Visibility,
		FCollisionShape::MakeSphere(TraceRadius),
		Params
	);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	if (bHit)
	{
		LOG_Art(Log, TEXT("[EmitLuxRay] ▶ 빛이 맞은 액터: %s"), *GetNameSafe(HitActor));
	}
	else
	{
		LOG_Art(Log, TEXT("[EmitLuxRay] ▶ 빛이 닿은 액터 없음"));
	}

	// 감지된 액터가 기믹 인터페이스 구현체면 트리거
	if (HitActor && HitActor->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
	{
		ILCGimmickInterface::Execute_ActivateGimmick(HitActor);
	}

	// 이전에 맞았던 액터가 다른 경우, Deactivate
	if (LastLitTarget && LastLitTarget != HitActor)
	{
		if (LastLitTarget->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(LastLitTarget);
		}
	}

	LastLitTarget = HitActor;

	// 클라이언트 이펙트 출력
	Multicast_EmitLightEffect(End);
}

void ALCRotationLuxStatue::DeactivateLux()
{
	if (!HasAuthority() || !bIsLuxActive) return;

	bIsLuxActive = false;

	LOG_Art(Log, TEXT("Lux 비활성화"));

	GetWorldTimerManager().ClearTimer(LuxEmitTimer);

	if (LastLitTarget && LastLitTarget->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
	{
		ILCGimmickInterface::Execute_DeactivateGimmick(LastLitTarget);
		LastLitTarget = nullptr;
	}

	if (LightEffectComponentLeft)
	{
		LightEffectComponentLeft->Deactivate();
	}

	if (LightEffectComponentRight)
	{
		LightEffectComponentRight->Deactivate();
	}
}

void ALCRotationLuxStatue::Multicast_EmitLightEffect_Implementation(const FVector& End)
{
	if (LightEffectTemplate)
	{
		if (LightEffectComponentLeft)
		{
			LightEffectComponentLeft->SetAsset(LightEffectTemplate);
			LightEffectComponentLeft->SetWorldRotation((End - LightEffectComponentLeft->GetComponentLocation()).Rotation());
			LightEffectComponentLeft->Activate(true);
		}
		if (LightEffectComponentRight)
		{
			LightEffectComponentRight->SetAsset(LightEffectTemplate);
			LightEffectComponentRight->SetWorldRotation((End - LightEffectComponentRight->GetComponentLocation()).Rotation());
			LightEffectComponentRight->Activate(true);
		}
	}

	DrawDebugLine(GetWorld(), LightOriginLeft->GetComponentLocation(), End, FColor::Yellow, false, 0.2f, 0, 2.f);
	DrawDebugLine(GetWorld(), LightOriginRight->GetComponentLocation(), End, FColor::Yellow, false, 0.2f, 0, 2.f);
}

void ALCRotationLuxStatue::Multicast_PlayLightSound_Implementation()
{
	if (LightActivateSound && AudioComponent)
	{
		AudioComponent->SetSound(LightActivateSound);
		AudioComponent->Play();
	}
}

void ALCRotationLuxStatue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCRotationLuxStatue, bIsLuxActive);
}
