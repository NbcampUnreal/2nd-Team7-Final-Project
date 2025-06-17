#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "BaseDrone.generated.h"

struct FInputActionValue;
class UDroneHUD;

UCLASS()
class LASTCANARY_API ABaseDrone : public APawn
{
	GENERATED_BODY()

public:
	///////////////////////////////////////////////////////////////////////////////
	//// Constructor & Overrides
	///////////////////////////////////////////////////////////////////////////////

	ABaseDrone();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	///////////////////////////////////////////////////////////////////////////////
	//// Components
	///////////////////////////////////////////////////////////////////////////////

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* DroneMesh;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* Camera;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	class UFloatingPawnMovement* MovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	class UItemSpawnerComponent* ItemSpawner;

	///////////////////////////////////////////////////////////////////////////////
	//// Movement Settings
	///////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = "Drone | Distance")
	float MaxDistanceToPlayer = 2000.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float MoveSpeed = 600.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalSpeed = 400.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float HorizontalAcceleration = 500.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float HorizontalDeceleration = 150.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalAcceleration = 300.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalDeceleration = 1.5f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float HorizontalMaxSpeed = 1200.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Movement")
	float VerticalMaxSpeed = 1200.f;

	///////////////////////////////////////////////////////////////////////////////
	//// Look Settings
	///////////////////////////////////////////////////////////////////////////////

	UPROPERTY(EditAnywhere, Category = "Drone | Look")
	float LookSensitivity = 100.f;

	UPROPERTY(EditAnywhere, Category = "Drone | Look")
	float RotationInterpSpeed = 5.f;

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

	///////////////////////////////////////////////////////////////////////////////
	//// Replication
	///////////////////////////////////////////////////////////////////////////////

	UPROPERTY(ReplicatedUsing = OnRep_CameraPitch)
	float CameraPitch = 0.f;

	UFUNCTION()
	void OnRep_CameraPitch();

	///////////////////////////////////////////////////////////////////////////////
	//// Input Handling
	///////////////////////////////////////////////////////////////////////////////

	void Input_Move(const FInputActionValue& Value);
	void Input_MoveUp(const FInputActionValue& Value);
	void Input_MoveDown(const FInputActionValue& Value);
	void Input_Look(const FInputActionValue& Value, float Sensivity);
	void Look(const FInputActionValue& Value);
	void Move(const FInputActionValue& Value);
	void MoveUp(const FInputActionValue& Value);
	void MoveDown(const FInputActionValue& Value);

	UFUNCTION(Server, Reliable)
	void Server_Move(FVector2D InputVector);
	void Server_Move_Implementation(FVector2D InputVector);

	UFUNCTION(Server, Reliable)
	void Server_MoveUp(float Value);
	void Server_MoveUp_Implementation(float Value);

	UFUNCTION(Server, Reliable)
	void Server_MoveDown(float Value);
	void Server_MoveDown_Implementation(float Value);

	UFUNCTION(Server, Reliable)
	void Server_Look(FVector2D InputVector);
	void Server_Look_Implementation(FVector2D InputVector);

	///////////////////////////////////////////////////////////////////////////////
	//// Drone Return Logic
	///////////////////////////////////////////////////////////////////////////////

	UFUNCTION(Server, Reliable)
	void Server_ReturnAsItem();
	void Server_ReturnAsItem_Implementation();
	void ReturnAsItem();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ReturnToPlayer();
	void Multicast_ReturnToPlayer_Implementation();

	UFUNCTION()
	void SpawnDroneItemAtCurrentLocation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Item")
	FName DroneItemRowName = FName("Drone");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|Item")
	FVector ItemSpawnOffset = FVector(0.0f, 0.0f, -50.0f);

	///////////////////////////////////////////////////////////////////////////////
	//// Utility
	///////////////////////////////////////////////////////////////////////////////

	FTimerHandle DroneDistanceTimerHandle;
	UDroneHUD* CachedDroneHUD = nullptr;
	float LastDistanceForHUD = -1.f;

	void UpdateDistanceCheck();

	UPROPERTY()
	FVector CharacterLocation;

	void SetCharacterLocation(FVector Location);

	UPROPERTY()
	FRotator TargetDroneRotation;

	UPROPERTY()
	FVector2D MoveInput;

	UPROPERTY()
	float VerticalInput;

	UPROPERTY()
	float VerticalInputAxis = 0.f;

	UPROPERTY()
	float VerticalVelocity = 0.f;

	UPROPERTY()
	FVector MoveDirection = FVector::ZeroVector;

	UPROPERTY()
	FVector CurrentVelocity = FVector::ZeroVector;

	UFUNCTION()
	void OnDroneHit(UPrimitiveComponent* HitComponent, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

protected:
	///////////////////////////////////////////////////////////////////////////////
	//// PostProcess Handling
	///////////////////////////////////////////////////////////////////////////////

	// 사용할 포스트 프로세스 머티리얼 배열
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Drone|PostProcess")
	TArray<UMaterialInterface*> PostProcessMaterials;

	// 현재 활성화된 머티리얼 인덱스
	UPROPERTY(VisibleAnywhere, Category = "Drone|PostProcess")
	int32 CurrentPPIndex = -1;

	// 드론 카메라에 적용할 다이나믹 머티리얼 (렌더링 시 사용)
	UPROPERTY()
	TArray<UMaterialInstanceDynamic*> DynamicPPInstances;

	// 클릭 시 호출되는 함수
	void TogglePostProcessEffect();

	// 서버에서 효과 전환 요청
	UFUNCTION(Server, Reliable)
	void Server_TogglePostProcessEffect();
	void Server_TogglePostProcessEffect_Implementation();

	// 실제 전환 로직 (서버 & 클라에서 공통 호출)
	void ApplyPostProcessMaterial(int32 NewIndex);
};