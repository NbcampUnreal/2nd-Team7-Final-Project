// Copyright Epic Games, Inc. All Rights Reserved.

#include "Weapon/Parts.h"

// Sets default values
AParts::AParts()
{
	PrimaryActorTick.bCanEverTick = false;

	Scene = CreateDefaultSubobject<USceneComponent>(TEXT("Scene"));
	SetRootComponent(Scene);

	PartsMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMesh"));
	PartsMesh->SetupAttachment(RootComponent);

	PartsType = "";
	SocketNameForAttach = "";
}

