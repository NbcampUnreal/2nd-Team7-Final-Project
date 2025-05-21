// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/Suppressor.h"

ASuppressor::ASuppressor()
{
	PartsType = "Suppresor";
	SocketNameForAttach = "MuzzleSocket";
}

FVector ASuppressor::GetMuzzleSocketLocation()
{
	return PartsMesh->GetSocketLocation("SuppressorMuzzleSocket");
}