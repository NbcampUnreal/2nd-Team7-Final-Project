// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseDrone.generated.h"

struct FInputActionValue;

UCLASS()
class LASTCANARY_API ABaseDrone : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseDrone();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


public: //Functions to process controller input.
	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneLookMouse(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneMoveUp(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneMoveRight(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneThrustFront(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneThrustBack(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneInteract(AActor* HitActor);

	UFUNCTION(BlueprintImplementableEvent)
	void Handle_DroneStrafe(const FInputActionValue& ActionValue);


public:
	UFUNCTION()
	void ReturnAsItem();
};
