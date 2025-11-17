// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Utility/CommonUtility.h"
#include "CharacterBaseComponent.generated.h"

class ABaseCharacter;
class UInGameHUD;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API UCharacterBaseComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UCharacterBaseComponent();
	
	UPROPERTY()
	bool bIsReady = false;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	

	UPROPERTY()
	APawn* CachedPawn;

	UPROPERTY()
	APlayerController* CachedController; 

	UPROPERTY()
	ACharacter* CachedCharacter;

	UPROPERTY()
	ABaseCharacter* CachedBaseCharacter;

	UPROPERTY()
	UAnimInstance* CachedAnimInstance;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	APawn* GetPawn() const { return CachedPawn; }
	ACharacter* GetCharacter() const { return CachedCharacter; }
	ABaseCharacter* GetBaseCharacter() const { return CachedBaseCharacter; }
	
	APlayerController* GetPlayerController() const { return CachedController; }
	
	UAnimInstance* GetCharacterAnimInstance() const { return CachedAnimInstance; }
	
	UInGameHUD* GetInGameHUD();
};
