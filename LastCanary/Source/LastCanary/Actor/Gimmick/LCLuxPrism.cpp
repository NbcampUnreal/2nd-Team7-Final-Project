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

	FVector Start = EmitOrigin->GetComponentLocation();
	FVector End = Start + EmitOrigin->GetForwardVector() * LightRange;

	if (bUseDebugLine)
	{
		DrawDebugLine(GetWorld(), Start, End, FColor::Cyan, false, EmitInterval, 0, 2.0f);
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
