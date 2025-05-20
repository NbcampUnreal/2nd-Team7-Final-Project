// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/SniperRifle.h"
#include "Weapon/Parts.h"

ASniperRifle::ASniperRifle()
{
	this->Damage = 50.0f;
	this->AttackRate = 1.5f;
	this->ReloadTime = 5.0f;
	this->WeaponType = "SniperRifle";
	this->OriginalAccuracy = 0.8f;
}

void ASniperRifle::EquipParts(AParts* Parts)
{
	if (Parts->ActorHasTag("SniperRifle"))
	{
		Super::EquipParts(Parts);
	}
}