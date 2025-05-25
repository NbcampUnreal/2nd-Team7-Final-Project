// Fill out your copyright notice in the Description page of Project Settings.


#include "Item/Drone/BaseDrone.h"

// Sets default values
ABaseDrone::ABaseDrone()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SetReplicatingMovement(true); // redundancy safety
	bReplicates = true;
}

// Called when the game starts or when spawned
void ABaseDrone::BeginPlay()
{
	Super::BeginPlay();
	
}
/*
void ABaseDrone::Handle_DroneLookMouse(const FInputActionValue& ActionValue)
{
	
}

void ABaseDrone::Handle_DroneMoveUp(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_DroneMoveRight(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_DroneThrustFront(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_DroneThrustBack(const FInputActionValue& ActionValue)
{

}

void ABaseDrone::Handle_DroneInteract(AActor* HitActor)
{

}

void ABaseDrone::Handle_DroneStrafe(const FInputActionValue& ActionValue)
{

}
*/

void ABaseDrone::ReturnAsItem()
{

	UE_LOG(LogTemp, Warning, TEXT("Drone return to Item"));
	//TODO: Spawn Drone Item Class
	Destroy();
}