#include "RuinsGimmickBase.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ARuinsGimmickBase::ARuinsGimmickBase()
	: RotationStep(45.f)
	, RotateSpeed(60.f)
	, bIsRotating(false)
	, TargetYaw(0.f)
	, CooldownTime(1.f)
	, LastActivatedTime(-1000.f)
	, InteractionMessage(TEXT("Press F"))
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);

	GimmickMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GimmickMesh"));
	SetRootComponent(GimmickMesh);
}

void ARuinsGimmickBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARuinsGimmickBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bIsRotating)
		return;

	const FRotator Current = GetActorRotation();
	const float NewYaw = FMath::FInterpConstantTo(Current.Yaw, TargetYaw, DeltaTime, RotateSpeed);
	SetActorRotation(FRotator(Current.Pitch, NewYaw, Current.Roll));

	if (FMath::IsNearlyEqual(NewYaw, TargetYaw, 0.1f))
	{
		SetActorRotation(FRotator(Current.Pitch, TargetYaw, Current.Roll));
		bIsRotating = false;
	}
}

void ARuinsGimmickBase::Interact_Implementation(APlayerController* Interactor)
{
	LOG_Art(Log, TEXT("▶ Interact_Implementation"));

	if (HasAuthority())
	{
		LOG_Art(Log, TEXT("▶ HasAuthority"));
		ActivateGimmick();
	}
	else
	{
		LOG_Art(Log, TEXT("▶ HasAuthority Not"));
		Server_Interact(Interactor);
	}
}

void ARuinsGimmickBase::Server_Interact_Implementation(APlayerController* Interactor)
{
	LOG_Art(Log, TEXT("▶ Server_Interact_Implementation "));
	ActivateGimmick();
}

FString ARuinsGimmickBase::GetInteractMessage_Implementation() const
{
	return 	TEXT("");
}

void ARuinsGimmickBase::ActivateGimmick_Implementation()
{
	const float Now = GetWorld()->GetTimeSeconds();
	if (Now - LastActivatedTime < CooldownTime)
		return;

	LastActivatedTime = Now;
	StartRotation(RotationStep);
	Multicast_PlaySound();
}

void ARuinsGimmickBase::StartRotation(float Step)
{
	const float CurrentYaw = GetActorRotation().Yaw;
	TargetYaw = FMath::Fmod(CurrentYaw + Step + 360.f, 360.f);
	bIsRotating = true;
}

void ARuinsGimmickBase::FinishRotation()
{
	bIsRotating = false;
}

void ARuinsGimmickBase::Multicast_PlaySound_Implementation()
{
	if (GimmickSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, GimmickSound, GetActorLocation());
	}
}

void ARuinsGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ARuinsGimmickBase, bIsRotating);
	DOREPLIFETIME(ARuinsGimmickBase, TargetYaw);
	DOREPLIFETIME(ARuinsGimmickBase, LastActivatedTime);
}