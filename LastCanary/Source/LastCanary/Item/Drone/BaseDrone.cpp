// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Drone/BaseDrone.h"

// Sets default values
ABaseDrone::ABaseDrone()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ABaseDrone::BeginPlay()
{
	Super::BeginPlay();
	
}


void ABaseDrone::Handle_LookMouse(const FInputActionValue& ActionValue)
{
	
}

void ABaseDrone::Handle_MoveUp(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_MoveRight(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_ThrustFront(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_ThrustBack(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_Interact(AActor* HitActor)
{

}

void ABaseDrone::Handle_Strafe(const FInputActionValue& ActionValue)
{

}