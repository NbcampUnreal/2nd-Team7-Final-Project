// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/Rifle.h"
#include "Weapon/Parts.h"

ARifle::ARifle()
{
	this->AttackRate = 0.09f;
	this->Damage = 17.0f;
	this->ReloadTime = 4.0f;
	this->WeaponType = "Rifle";
	this->OriginalAccuracy = 0.65f;
}

void ARifle::EquipParts(AParts* Parts)
{
	if (Parts->ActorHasTag("Rifle"))
	{
		Super::EquipParts(Parts);
	}
}