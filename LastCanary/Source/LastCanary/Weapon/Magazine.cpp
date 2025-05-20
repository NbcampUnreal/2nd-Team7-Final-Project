// Copyright Epic Games, Inc. All Rights Reserved.

#include "Magazine.h"
#include "Weapon/Firearm.h"

AMagazine::AMagazine()
{
	PartsType = "Magazine";
	SocketNameForAttach = "MagazineSocket";
	MagazineCapacity = 30;
}

int32 AMagazine::GetMagazineCapacity() const
{
	return MagazineCapacity;
}