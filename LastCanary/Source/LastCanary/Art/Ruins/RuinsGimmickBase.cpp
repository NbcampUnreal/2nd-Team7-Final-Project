#include "RuinsGimmickBase.h"
#include "Net/UnrealNetwork.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "LastCanary.h"

ARuinsGimmickBase::ARuinsGimmickBase()
{
	PrimaryActorTick.bCanEverTick = true;

	GimmickMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GimmickMesh"));
	RootComponent = GimmickMesh;

	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(true);

}

void ARuinsGimmickBase::BeginPlay()
{
	Super::BeginPlay();
}

void ARuinsGimmickBase::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void ARuinsGimmickBase::Interact_Implementation(APlayerController* Interactor)
{
	LOG_Art(Log, TEXT("▶ Interact_Implementation 호출 - NetRole: %s, HasAuthority: %s, Interactor: %s"),
		*UEnum::GetValueAsString(GetLocalRole()),
		HasAuthority() ? TEXT("true") : TEXT("false"),
		*GetNameSafe(Interactor));

	if (HasAuthority())
	{
		ActivateGimmick();
	}
}

FString ARuinsGimmickBase::GetInteractMessage_Implementation() const
{
	return InteractionMessage;
}

void ARuinsGimmickBase::ActivateGimmick_Implementation()
{
	LOG_Art(Log, TEXT("▶ ActivateGimmick_Implementation 호출"));
	Multicast_PlaySound();

}

void ARuinsGimmickBase::Multicast_PlaySound_Implementation()
{
	if (GimmickSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, GimmickSound, GetActorLocation());
	}
	else
	{
		LOG_Art_WARNING(TEXT("GimmickSound가 지정되지 않았습니다."));
	}
}

void ARuinsGimmickBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}
