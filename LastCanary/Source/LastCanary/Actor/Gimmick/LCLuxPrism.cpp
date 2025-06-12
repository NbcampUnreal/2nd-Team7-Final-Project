#include "Actor/Gimmick/LCLuxPrism.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "Components/SceneComponent.h"
#include "Components/AudioComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "LastCanary.h"

ALCLuxPrism::ALCLuxPrism()
	: bIsLuxReceived(false)
	, LightRange(1000.f)
	, bUseDebugLine(true)
	, EmitInterval(0.1f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	EmitOrigin = CreateDefaultSubobject<USceneComponent>(TEXT("EmitOrigin"));
	EmitOrigin->SetupAttachment(RootComponent);

	NiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("NiagaraComponent"));
	NiagaraComponent->SetupAttachment(EmitOrigin);
	NiagaraComponent->SetAutoActivate(false);

	AudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("AudioComponent"));
	AudioComponent->SetupAttachment(EmitOrigin);
	AudioComponent->SetAutoActivate(false);
}

void ALCLuxPrism::BeginPlay()
{
	Super::BeginPlay();
}

void ALCLuxPrism::ActivateGimmick_Implementation()
{
	LOG_Art(Log, TEXT("▶ ALCLuxPrism::ActivateGimmick_Implementation 호출됨"));

	if (!HasAuthority())
	{
		Server_ActivateGimmick();
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetOwner());

	if (IsValid(PC))
	{
		Super::ActivateGimmick_Implementation();
	}
	else
	{
		if (!bIsLuxReceived)
		{
			IGimmickEffectInterface::Execute_TriggerEffect(this);
		}
	}
}

void ALCLuxPrism::TriggerEffect_Implementation()
{
	if (bIsLuxReceived) return;

	bIsLuxReceived = true;
	StartEmitLux();
	Multicast_StartEmitLux();
}

void ALCLuxPrism::StopEffect_Implementation()
{
	if (!bIsLuxReceived) return;

	LOG_Art(Log, TEXT("▶ StopEffect_Implementation 호출됨"));

	bIsLuxReceived = false;

	Multicast_StopEmitLux();
	StopEmitLux();
}

void ALCLuxPrism::StartEmitLux()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().SetTimer(EmitTimerHandle, this, &ALCLuxPrism::EmitLux, EmitInterval, true);
	}

	if (EmitEffect && NiagaraComponent)
	{
		NiagaraComponent->SetAsset(EmitEffect);
		NiagaraComponent->Activate(true);
		LOG_Art(Log, TEXT("▶ StartEmitLux - NiagaraComponent 활성화됨"));
	}

	if (EmitSound && AudioComponent)
	{
		AudioComponent->SetSound(EmitSound);
		AudioComponent->Play();
		LOG_Art(Log, TEXT("▶ StartEmitLux - AudioComponent 재생됨"));
	}

	if (NiagaraComponent)
	{
		const FVector Location = NiagaraComponent->GetComponentLocation();
		const FRotator Rotation = NiagaraComponent->GetComponentRotation();

		LOG_Art(Log, TEXT("[StartEmitLux] ▶ NiagaraComponent 위치: %s / 회전: %s"),
			*Location.ToString(), *Rotation.ToString());

		if (!NiagaraComponent->IsActive())
		{
			LOG_Art_WARNING(TEXT("[StartEmitLux] ▶ NiagaraComponent가 비활성 상태임"));
		}
	}
	else
	{
		LOG_Art_ERROR(TEXT("[StartEmitLux] ▶ NiagaraComponent 없음!"));
	}

}

void ALCLuxPrism::StopEmitLux()
{
	if (HasAuthority())
	{
		GetWorld()->GetTimerManager().ClearTimer(EmitTimerHandle);
	}

	if (NiagaraComponent)
	{
		NiagaraComponent->Deactivate();
		LOG_Art(Log, TEXT("▶ StopEmitLux - NiagaraComponent 비활성화됨"));
	}

	if (AudioComponent)
	{
		AudioComponent->Stop();
		LOG_Art(Log, TEXT("▶ StopEmitLux - AudioComponent 중지됨"));
	}
}

void ALCLuxPrism::EmitLux()
{
	if (!EmitOrigin) return;

	const FVector Start = EmitOrigin->GetComponentLocation();
	const FVector End = Start + EmitOrigin->GetForwardVector() * LightRange;

	// Sweep 트레이스 (원기둥 형태, 통과 없음)
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const float TraceRadius = 100.f;

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
		LOG_Art(Log, TEXT("[EmitLux] ▶ 빛이 맞은 액터: %s"), *GetNameSafe(HitActor));

		if (HitActor->ActorHasTag("Lux"))
		{
			if (HitActor->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
			{
				ILCGimmickInterface::Execute_ActivateGimmick(HitActor);
			}
		}
	}
	else
	{
		LOG_Art(Log, TEXT("[EmitLux] ▶ 빛이 닿은 액터 없음"));
	}

	// 디버그 라인은 맞은 곳까지만
	const FVector VisualEnd = bHit ? Hit.ImpactPoint : End;
	if (bUseDebugLine)
	{
		DrawDebugLine(GetWorld(), Start, VisualEnd, FColor::Cyan, false, EmitInterval + 0.05f, 0, 2.f);
		LOG_Art(Log, TEXT("[EmitLux] ▶ 디버그 라인: %s → %s"), *Start.ToString(), *VisualEnd.ToString());
	}
}

void ALCLuxPrism::Multicast_StartEmitLux_Implementation()
{
	if (!HasAuthority())
	{
		StartEmitLux();
	}
}

void ALCLuxPrism::Multicast_StopEmitLux_Implementation()
{
	LOG_Art(Log, TEXT("▶ Multicast_StopEmitLux_Implementation 호출됨"));

	if (!HasAuthority())
	{
		StopEmitLux();
	}
}

void ALCLuxPrism::DeactivateGimmick_Implementation()
{
	LOG_Art(Log, TEXT("▶ DeactivateGimmick_Implementation 호출됨"));

	IGimmickEffectInterface::Execute_StopEffect(this);
}
