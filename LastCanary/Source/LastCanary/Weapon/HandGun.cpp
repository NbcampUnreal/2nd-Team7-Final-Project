// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/HandGun.h"
#include "Weapon/Parts.h"

AHandGun::AHandGun()
{
	this->Damage = 10.0f;
	this->AttackRate = 0.11f;
	this->ReloadTime = 3.0f;
	this->OriginalAccuracy = 0.7f;
	this->WeaponType = "HandGun";
}

void AHandGun::EquipParts(AParts* Parts)
{
	if (Parts->ActorHasTag("HandGun"))
	{
		Super::EquipParts(Parts);
	}
}