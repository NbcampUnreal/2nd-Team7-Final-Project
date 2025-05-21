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
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue);

	virtual void Handle_MoveUp(const FInputActionValue& ActionValue);

	virtual void Handle_MoveRight(const FInputActionValue& ActionValue);

	virtual void Handle_ThrustFront(const FInputActionValue& ActionValue);

	virtual void Handle_ThrustBack(const FInputActionValue& ActionValue);

	virtual void Handle_Interact(AActor* HitActor);

	virtual void Handle_Strafe(const FInputActionValue& ActionValue);
};
