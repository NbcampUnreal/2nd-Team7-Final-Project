#include "LCRotationLuxStatue.h"
#include "LCRotationLuxStatue.h"
#include "Interface/LCGimmickInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "DrawDebugHelpers.h"
#include "LastCanary.h"

ALCRotationLuxStatue::ALCRotationLuxStatue()
	: bIsLuxActive(false)
	, LightRange(2000.f)
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;

	LightOriginMain = CreateDefaultSubobject<USceneComponent>(TEXT("LightOriginMain"));
	LightOriginMain->SetupAttachment(VisualMesh);
	LightOriginMain->SetRelativeLocation(FVector(100.f, 0.f, 50.f));

	LightOriginLeft = CreateDefaultSubobject<USceneComponent>(TEXT("LightOriginLeft"));
	LightOriginLeft->SetupAttachment(VisualMesh);
	LightOriginLeft->SetRelativeLocation(FVector(100.f, -30.f, 50.f));

	LightOriginRight = CreateDefaultSubobject<USceneComponent>(TEXT("LightOriginRight"));
	LightOriginRight->SetupAttachment(VisualMesh);
	LightOriginRight->SetRelativeLocation(FVector(100.f, 30.f, 50.f));
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
	if (!HasAuthority() || bIsLuxActive)
	{
		return;
	}

	bIsLuxActive = true;

	LOG_Art(Log, TEXT("Lux 활성화"));

	if (LightActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LightActivateSound, GetActorLocation());
	}

	Multicast_PlayLightSound();

	GetWorldTimerManager().SetTimer(LuxEmitTimer, this, &ALCRotationLuxStatue::EmitLuxRay, 0.2f, true);
}

void ALCRotationLuxStatue::EmitLuxRay()
{
	if (!HasAuthority() || !bIsLuxActive)
	{
		return;
	}

	// 트레이스는 메인 포인트 기준
	const FVector Start = LightOriginMain->GetComponentLocation();
	const FVector End = Start + LightOriginMain->GetForwardVector() * LightRange;

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params);

	if (bHit && Hit.GetActor() && Hit.GetActor()->GetClass()->ImplementsInterface(ULCGimmickInterface::StaticClass()))
	{
		ILCGimmickInterface::Execute_ActivateGimmick(Hit.GetActor());
	}

	const FVector VisualLeftStart = LightOriginLeft->GetComponentLocation();
	const FVector VisualRightStart = LightOriginRight->GetComponentLocation();
	const FVector VisualEnd = End;

	Multicast_EmitLightEffect(VisualLeftStart, VisualEnd);
	Multicast_EmitLightEffect(VisualRightStart, VisualEnd);
}

bool ALCRotationLuxStatue::IsLuxActive() const
{
	return bIsLuxActive;
}

void ALCRotationLuxStatue::DeactivateLux()
{
	if (!HasAuthority() || !bIsLuxActive)
	{
		return;
	}

	bIsLuxActive = false;

	LOG_Art(Log, TEXT("Lux 비활성화"));

	GetWorldTimerManager().ClearTimer(LuxEmitTimer);
}

void ALCRotationLuxStatue::Multicast_EmitLightEffect_Implementation(const FVector& Start, const FVector& End)
{
	DrawDebugLine(GetWorld(), Start, End, FColor::Yellow, false, 0.2f, 0, 2.f);
}

void ALCRotationLuxStatue::Multicast_PlayLightSound_Implementation()
{
	if (LightActivateSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, LightActivateSound, GetActorLocation());
	}
}

void ALCRotationLuxStatue::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ALCRotationLuxStatue, bIsLuxActive);
}