// Copyright Epic Games, Inc. All Rights Reserved.

#include "Fist.h"
#include "Components/SphereComponent.h"

AFist::AFist()
{
	FistCoillision = CreateDefaultSubobject<USphereComponent>(TEXT("SphereCollision"));
	FistCoillision->SetupAttachment(RootComponent);
	FistCoillision->InitSphereRadius(10.0f);
	FistCoillision->SetCollisionProfileName(TEXT("OverlapAllDynamic"));

	if (WeaponMesh)
	{
		WeaponMesh->SetSkeletalMesh(nullptr);
	}

	this->Damage = 5.0f;
	this->AttackRate = 1.0f;
	this->WeaponType = "Fist";
}

void AFist::Attack()
{
	Super::Attack();
	if (FistCoillision)
	{
		FistCoillision->OnComponentBeginOverlap.AddDynamic(this, &AFist::OnEnemyOverlap);
	}
}