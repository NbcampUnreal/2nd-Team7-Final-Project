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
	EmitOrigin->SetUsingAbsoluteRotation(true);
	EmitOrigin->SetRelativeLocation(FVector::ZeroVector);
	EmitOrigin->SetWorldRotation(FRotator(0.f, 90.f, 0.f)); 

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
	if (bIsLuxReceived) return;

	LOG_Art(Log, TEXT("ALCLuxPrism::ActivateGimmick_Implementation 호출됨"));

	if (!HasAuthority())
	{
		Server_ActivateGimmick();
		return;
	}

	IGimmickEffectInterface::Execute_TriggerEffect(this);
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

	bIsLuxReceived = false;
	StopEmitLux();
	Multicast_StopEmitLux();
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
	}

	if (EmitSound && AudioComponent)
	{
		AudioComponent->SetSound(EmitSound);
		AudioComponent->Play();
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
	}

	if (AudioComponent)
	{
		AudioComponent->Stop();
	}
}

void ALCLuxPrism::EmitLux()
{
	if (!EmitOrigin) return;

	const FVector Start = EmitOrigin->GetComponentLocation();
	const FVector Direction = EmitOrigin->GetForwardVector().GetSafeNormal();
	const FVector End = Start + Direction * LightRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	const float TraceRadius = 100.f;

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	if (bHit && HitActor && HitActor->ActorHasTag("Lux"))
	{
		if (HitActor->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
		{
			ILCGimmickInterface::Execute_ActivateGimmick(HitActor);
		}
	}

	if (bUseDebugLine)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, EmitInterval + 0.05f, 0, 2.f);
		//LOG_Art(Log, TEXT("[EmitLux] 예상 방향 라인: %s → %s"), *Start.ToString(), *End.ToString());

		if (bHit)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.f, 12, FColor::Red, false, EmitInterval + 0.05f);
			//LOG_Art(Log, TEXT("[EmitLux] 충돌 지점: %s | 맞은 액터: %s"), *Hit.ImpactPoint.ToString(), *GetNameSafe(HitActor));
		}
		else
		{
			//LOG_Art(Log, TEXT("[EmitLux] 충돌 없음"));
		}
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
	if (!HasAuthority())
	{
		StopEmitLux();
	}
}

void ALCLuxPrism::DeactivateGimmick_Implementation()
{
	IGimmickEffectInterface::Execute_StopEffect(this);
}