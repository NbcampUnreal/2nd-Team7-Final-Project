#include "AI/CaveEliteGimmickComponent.h"
#include "AI/EliteMonster/CaveEliteMonster.h"
#include "TimerManager.h"

UCaveEliteGimmickComponent::UCaveEliteGimmickComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	InitSphereRadius(400.0f);
	SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetCollisionResponseToAllChannels(ECR_Ignore);
	SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	SetHiddenInGame(true);
}

void UCaveEliteGimmickComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bAutoActivateGimmick)
	{
		SetGimmickActive(true);
	}
}

void UCaveEliteGimmickComponent::SetGimmickActive(bool bNewActive)
{
	if (bNewActive)
	{
		SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		SetGenerateOverlapEvents(true);

		if (GetOwner() && GetOwner()->HasAuthority())
		{
			GetWorld()->GetTimerManager().SetTimer(
				OverlapCheckTimer,
				this,
				&UCaveEliteGimmickComponent::CheckOverlap,
				0.5f,
				true
			);
		}
	}
	else
	{
		SetCollisionEnabled(ECollisionEnabled::NoCollision);
		SetGenerateOverlapEvents(false);

		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(OverlapCheckTimer);
		}
	}
}

void UCaveEliteGimmickComponent::CheckOverlap()
{
	if (!GetOwner() || !GetOwner()->HasAuthority())
	{
		return;
	}

	TArray<AActor*> OverlappingActors;

	GetOverlappingActors(OverlappingActors);

	for (AActor* Actor : OverlappingActors)
	{
		if (ACaveEliteMonster* EliteMonster = Cast<ACaveEliteMonster>(Actor))
		{
			EliteMonster->FreezeAI();
		}
	}
}