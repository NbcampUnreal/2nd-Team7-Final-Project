// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BaseSpectatorPawn.generated.h"

struct FInputActionValue;
class ABaseCharacter;

UCLASS()
class LASTCANARY_API ABaseSpectatorPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ABaseSpectatorPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	void Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity);
	void AdjustCameraZoom(float ZoomDelta); // 선택사항

	void SpectateOtherUser(ABaseCharacter* TargetCharacter);


	virtual void Handle_VoiceChatting(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void StartVoiceChat();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void CancelVoiceChat();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* Camera;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float DefaultZoom = 300.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinZoom = 200.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxZoom = 400.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MinPitchAngle = 10.f;

	UPROPERTY(EditAnywhere, Category = "Camera")
	float MaxPitchAngle = 80.0f;
};
