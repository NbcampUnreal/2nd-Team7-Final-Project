#include "RuinsRotatingStatue.h"
#include "Components/StaticMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "LastCanary.h"

ARuinsRotatingStatue::ARuinsRotatingStatue()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	SetReplicateMovement(true);
	bAlwaysRelevant = true;

	CooldownTime = 0.f;
	LastActivatedTime = -1000.f;
}

void ARuinsRotatingStatue::BeginPlay()
{
	Super::BeginPlay();

	LOG_Art(Log, TEXT("▶ ARuinsRotatingStatue::BeginPlay - 동상 초기화 완료"));
}

void ARuinsRotatingStatue::Interact_Implementation(APlayerController* Interactor)
{
	LOG_Art(Log, TEXT("▶ ARuinsRotatingStatue::Interact_Implementation - Role: %s"),
		*UEnum::GetValueAsString(GetLocalRole()));

	if (HasAuthority())
	{
		ActivateGimmick();  
	}
	else
	{
		Server_Interact(Interactor);  
	}
}

void ARuinsRotatingStatue::ActivateGimmick_Implementation()
{
	StartRotation(RotationStep);
	Multicast_PlaySound();

	LOG_Art(Log, TEXT("▶ ARuinsRotatingStatue::ActivateGimmick_Implementation - 회전 시작"));
}