#include "Actor/Gimmick/LCLuxPrism.h"
#include "Actor/Gimmick/Trigger/LCLuxChargeTrigger.h"
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

	//LOG_Art(Log, TEXT("ALCLuxPrism::ActivateGimmick_Implementation 호출됨"));

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
		//LOG_Art(Log, TEXT("[서버] EmitLux 타이머 시작"));
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
	//LOG_Art(Log, TEXT("▶ EmitLux 실행 중"));

	if (!EmitOrigin) return;

	const FVector Start = EmitOrigin->GetComponentLocation();
	const FVector Direction = EmitOrigin->GetForwardVector().GetSafeNormal();
	const FVector End = Start + Direction * LightRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit,
		Start,
		End,
		ECC_Visibility,
		Params
	);

	AActor* HitActor = bHit ? Hit.GetActor() : nullptr;

	if (HasAuthority())
	{
		if (bHit && HitActor && HitActor->ActorHasTag("Lux"))
		{
			if (HitActor->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
			{
				IGimmickEffectInterface::Execute_TriggerEffect(HitActor);
				//LOG_Art(Log, TEXT("[EmitLux][Server] ▶ TriggerEffect 호출: %s"), *HitActor->GetName());
			}
		}
	}

	if (bUseDebugLine)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, EmitInterval + 0.05f, 0, 2.f);

		if (bHit)
		{
			DrawDebugSphere(GetWorld(), Hit.ImpactPoint, 16.f, 12, FColor::Red, false, EmitInterval + 0.05f);
		}
	}
}

void ALCLuxPrism::Multicast_StartEmitLux_Implementation()
{
	if (!HasAuthority())
	{
		//LOG_Art(Log, TEXT("[클라] Multicast_StartEmitLux_Implementation 호출됨"));
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