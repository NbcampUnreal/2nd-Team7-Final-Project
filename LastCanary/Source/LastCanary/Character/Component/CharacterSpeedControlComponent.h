// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterSpeedControlComponent.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API UCharacterSpeedControlComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
	UCharacterSpeedControlComponent();

	////////* Default Stats *////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultWalkSpeed = 175.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultRunSpeed = 375.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultSprintSpeed = 650.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultCrouchSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultJumpZVelocity = 450.0f;

	////////* InGame Stats *////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WalkSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float RunSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SprintSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CrouchSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float JumpZVelocity;

	void InitIngameMovementSpeed();
protected:
	virtual void BeginPlay() override;

	UFUNCTION(Client, Reliable)
	void Client_SetMovementSetting();
	void Client_SetMovementSetting_Implementation();

public:
	void SetCharacterMovementSpeed();
};
