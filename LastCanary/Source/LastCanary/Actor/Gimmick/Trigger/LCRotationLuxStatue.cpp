#include "LCRotationLuxStatue.h"
#include "Actor/Gimmick/LCLuxPrism.h"
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
	LightOriginLeft->SetRelativeLocation(FVector(100.f, 0.f, 50.f));

	LightEffectComponentLeft = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LightEffectLeft"));
	LightEffectComponentLeft->SetupAttachment(LightOriginLeft);
	LightEffectComponentLeft->SetAutoActivate(false);

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

	//LOG_Art(Log, TEXT("Lux 활성화"));

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

	if (bHit && HitActor && HitActor->ActorHasTag("Lux"))
	{
		if (HitActor->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
		{
			if (ALCLuxPrism* Prism = Cast<ALCLuxPrism>(HitActor))
			{
				Prism->TriggerEffectFrom(this); 
			}
			else
			{
				ILCGimmickInterface::Execute_ActivateGimmick(HitActor);
			}
		}
	}

	if (LastLitTarget && LastLitTarget != HitActor &&
		LastLitTarget->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
	{
		if (ALCLuxPrism* Prism = Cast<ALCLuxPrism>(LastLitTarget))
		{
			Prism->StopEffectFrom(this);
		}
		else
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(LastLitTarget);
		}
	}

	LastLitTarget = HitActor;

	const FVector VisualEnd = bHit ? Hit.ImpactPoint : End;
	Multicast_EmitLightEffect(VisualEnd);
}

void ALCRotationLuxStatue::DeactivateLux()
{
	if (!HasAuthority() || !bIsLuxActive) return;

	bIsLuxActive = false;
	GetWorldTimerManager().ClearTimer(LuxEmitTimer);

	if (LastLitTarget && LastLitTarget->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
	{
		if (ALCLuxPrism* Prism = Cast<ALCLuxPrism>(LastLitTarget))
		{
			Prism->StopEffectFrom(this); 
		}
		else
		{
			ILCGimmickInterface::Execute_DeactivateGimmick(LastLitTarget);
		}
		LastLitTarget = nullptr;
	}

	Multicast_StopLightEffect();
}

void ALCRotationLuxStatue::Multicast_EmitLightEffect_Implementation(const FVector& End)
{
	if (LightEffectTemplate && LightEffectComponentLeft)
	{
		LightEffectComponentLeft->SetAsset(LightEffectTemplate);
		LightEffectComponentLeft->SetWorldRotation((End - LightEffectComponentLeft->GetComponentLocation()).Rotation());
		LightEffectComponentLeft->Activate(true);
	}

	if (bUseDebugLine)
	{
		DrawDebugLine(GetWorld(), LightOriginLeft->GetComponentLocation(), End, FColor::Yellow, false, 0.2f, 0, 2.f);
	}
}

void ALCRotationLuxStatue::Multicast_PlayLightSound_Implementation()
{
	if (LightActivateSound && AudioComponent)
	{
		AudioComponent->SetSound(LightActivateSound);
		AudioComponent->Play();
	}
}

void ALCRotationLuxStatue::Multicast_StopLightEffect_Implementation()
{
	if (LightEffectComponentLeft)
	{
		LightEffectComponentLeft->Deactivate();
	}
}

void ALCRotationLuxStatue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALCRotationLuxStatue, bIsLuxActive);
}
