// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
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
	virtual void BeginPlay() override;

	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
public:

	FVector CharacterLocation;
	void SetCharacterLocation(FVector Location);

	UPROPERTY(EditAnywhere, Category = "Drone | Distance")
	float MaxDistanceToPlayer= 2000.f;

	// 충돌 이벤트 함수 선언 (OnComponentHit 시그니처)
	UFUNCTION()
	void OnDroneHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

public:
	virtual void Tick(float DeltaTime) override;
	
		
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DroneMesh;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* MovementComponent;

	// 상하 카메라 회전값 (Pitch)
	UPROPERTY(ReplicatedUsing = OnRep_CameraPitch)
	float CameraPitch = 0.f;

	UFUNCTION()
	void OnRep_CameraPitch();

	// Movement input
	FVector2D MoveInput; // X: Forward/Back, Y: Right/Left
	float VerticalInput;

	// 수직 속도
	float VerticalVelocity = 0.f;

	// 상승 / 하강 입력 상태
	float VerticalInputAxis = 0.f;

	// Settings
	UPROPERTY(EditAnywhere)
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere)
	float VerticalSpeed = 400.f;

	FRotator TargetDroneRotation;

	UPROPERTY(EditAnywhere, Category = "Drone | Look")
	float LookSensitivity = 100.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Look")
	float RotationInterpSpeed = 5.f;


	void Input_Move(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void Server_Move(FVector2D InputVector);
	void Server_Move_Implementation(FVector2D InputVector);
	void Move(const FInputActionValue& Value);


	void Input_MoveUp(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void Server_MoveUp(float Value);
	void Server_MoveUp_Implementation(float Value);
	void MoveUp(const FInputActionValue& Value);



	void Input_MoveDown(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void Server_MoveDown(float Value);
	void Server_MoveDown_Implementation(float Value);
	void MoveDown(const FInputActionValue& Value);

	void Input_Look(const FInputActionValue& Value);
	UFUNCTION(Server, Reliable)
	void Server_Look(FVector2D InputVector);
	void Server_Look_Implementation(FVector2D InputVector);
	void Look(const FInputActionValue& Value);

	// 현재 속도 벡터
	FVector CurrentVelocity = FVector::ZeroVector;

	// 움직임 목표 방향 (Y=앞뒤, X=좌우), VerticalInput은 그대로 둠
	FVector MoveDirection = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float HorizontalAcceleration = 500.0;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float HorizontalDeceleration = 150.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalAcceleration = 300.0f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalDeceleration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float HorizontalMaxSpeed = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalMaxSpeed = 1200.f;

protected:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|MouseSensitivity", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookUpMouseSensitivity{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|MouseSensitivity", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookRightMouseSensitivity{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|MouseSensitivity", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookUpRate{ 90.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|MouseSensitivity", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookRightRate{ 240.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float MaxPitchAngle{ 60.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Meta = (ClampMin = -80, ClampMax = 0, ForceUnits = "deg"))
	float MinPitchAngle{ -60.0f };




public:
	UFUNCTION(Server, Reliable)
	void Server_ReturnAsItem();

	void Server_ReturnAsItem_Implementation();
	void ReturnAsItem();


public:
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReturnToPlayer();
	void Multicast_ReturnToPlayer_Implementation();
};
