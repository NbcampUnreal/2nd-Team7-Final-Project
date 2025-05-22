// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/SubMachineGun.h"
#include "Weapon/Parts.h"

ASubMachineGun::ASubMachineGun()
{
	this->AttackRate = 0.05f;
	this->Damage = 5.0f;
	this->ReloadTime = 4.0f;
	this->WeaponType = "SMG";
	this->OriginalAccuracy = 0.5f;
}

void ASubMachineGun::EquipParts(AParts* Parts)
{
	if (Parts->ActorHasTag("SMG"))
	{
		Super::EquipParts(Parts);
	}
}