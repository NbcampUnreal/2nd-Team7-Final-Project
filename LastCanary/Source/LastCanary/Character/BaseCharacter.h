#pragma once

#include "CoreMinimal.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsCharacter.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "BaseCharacter.generated.h"

struct FInputActionValue;
class UAlsCameraComponent;
class UInputMappingContext;
class UInputAction;
class UBoxComponent;
class USpringArmComponent;
class UCameraComponent;
class AItemBase;
class UToolbarInventoryComponent;
class UBackpackInventoryComponent;
struct FBaseItemSlotData;
class UItemSpawnerComponent;

UCLASS()
class LASTCANARY_API ABaseCharacter : public AAlsCharacter
{
	GENERATED_BODY()

	//Character Mesh and Component
public:
	/*1인칭 전용 메시 (자신만 보이는)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	UStaticMeshComponent* OverlayStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* OverlaySkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* RemoteOnlySkeletalMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	UStaticMeshComponent* RemoteOnlyOverlayStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* RemoteOnlyOverlaySkeletalMesh;


	// SpringArm 컴포넌트 (카메라 거리와 회전 보정용)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	// Camera 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* Camera;

	// SpringArm 컴포넌트 for ADS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* ADSSpringArm; //Aim Down Sight

	// Camera 컴포넌트 for ADS
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* ADSCamera;
	// 캐릭터 인벤토리 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UToolbarInventoryComponent* ToolbarInventoryComponent;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UBackpackInventoryComponent* BackpackInventoryComponent;


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> ThirdPersonArrow;


	// 관전용 스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpectatorSpringArm;

	// 관전용 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* SpectatorCamera;


	void Tick(float DeltaSeconds)
	{
		Super::Tick(DeltaSeconds);

		//SmoothADSCamera(DeltaSeconds);
		UpdateGunWallClipOffset(DeltaSeconds);
	}
	float WallClipAimOffsetPitch;
	float MaxWallClipPitch = 90.0f;
	float CapsuleWallRatio = 0.0f;
	void UpdateGunWallClipOffset(float DeltaTime);

	//Character Default Settings
protected:
	/*Character Default Settings*/
	ABaseCharacter();
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
	virtual void NotifyControllerChanged() override;
	virtual void BeginPlay() override;


	// Camera Settings
protected:
	/*Camera Settings*/
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

	void CalcCameraLocation();

	void CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo);

	FTimerHandle MoveTimerHandle;
	FVector StartLocation;
	FVector TargetLocation;
	FName TargetSocketName = FName("");
	float InterpSpeed = 15.0f;
	float SnapTolerance = 1.0f;
	void StartSmoothMove(const FVector& Start, const FVector& Destination);
	void SmoothMoveStep();

	bool bIsFPSCamera = true;
	void ToADSCamera(bool bToADS);
	bool bDesiredADS = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothCameraSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothCameraTimeThreshold = 0.5f;

	float SmoothCameraCurrentTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.f;

	void SmoothADSCamera(float DeltaTime);
	bool bADS = false; // 현재 정조준 상태인가?



public:
	void SetCameraMode(bool bIsFirstPersonView);


	// Character Input Handle Function

public:
	/*Function called by the controller*/
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue);
	virtual void Handle_Look(const FInputActionValue& ActionValue);
	virtual void Handle_Move(const FInputActionValue& ActionValue);
	virtual void Handle_Sprint(const FInputActionValue& ActionValue);
	virtual void Handle_Walk(const FInputActionValue& ActionValue);
	virtual void Handle_Crouch(const FInputActionValue& ActionValue);
	virtual void Handle_Jump(const FInputActionValue& ActionValue);
	virtual void Handle_Strafe(const FInputActionValue& ActionValue);
	virtual void Handle_Aim(const FInputActionValue& ActionValue);
	virtual void Handle_Interact();
	virtual void Handle_ViewMode();
	virtual void Handle_Reload();

	void EscapeThroughGate();

	//Character State

public:
	bool bIsScoped = false;
	bool bIsPossessed;
	bool bIsReloading = false;
	bool bIsClose = false;
	bool bIsSprinting = false;
	void SetPossess(bool IsPossessed);


	//About Character Animation Montage and Animation Class
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool UseGunBoneforOverlayObjects;

	UFUNCTION(BlueprintCallable)
	void RefreshOverlayObject(int index);

	UFUNCTION(BlueprintCallable)
	void AttachOverlayObject(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, TSubclassOf<UAnimInstance> NewAnimationClass, FName SocketName, bool bUseLeftGunBone);

	UFUNCTION(BlueprintCallable)
	void RefreshOverlayLinkedAnimationLayer(int index);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> RifleAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> PistolAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> TorchAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	USkeletalMesh* SKM_Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	USkeletalMesh* SKM_Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UStaticMesh* SM_Torch;


public:
	//애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* InteractMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* ReloadMontage;

	UFUNCTION()
	void OnGunReloadAnimComplete(UAnimMontage* CompletedMontage, bool bInterrupted);

	UFUNCTION()
	void PlayInteractionMontage(AActor* Target);

	UFUNCTION(Server, Reliable)
	void Server_PlayMontage(UAnimMontage* MontageToPlay);
	void Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayMontage(UAnimMontage* MontageToPlay);
	void Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay);



	//Check Player Focus Everytime
public:
	/*About Interact*/

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Interaction")
	float TraceDistance = 300.0f;

	FTimerHandle InteractionTraceTimerHandle;

	// 현재 바라보고 있는 상호작용 가능한 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
	AActor* CurrentFocusedActor;

	void TraceInteractableActor();


	//Player Take Damage
public:
	/*Player Damage, Death*/
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void HandlePlayerDeath();
	
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerInGameStateOnDie();
	void Multicast_SetPlayerInGameStateOnDie_ImplementationOnDie();

	virtual void GetFallDamage(float Velocity) override;

	float CalculateTakeDamage(float DamageAmount);
	float CalculateFallDamage(float Velocity);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerInGameStateOnEscapeGate();
	void Multicast_SetPlayerInGameStateOnEscapeGateImplementationOnDie();


	UFUNCTION(Server, Reliable)
	void Server_PlayReload();
	void Server_PlayReload_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReload();
	void Multicast_PlayReload_Implementation();


public:

	void PickupItem();

	// 아이템 퀵슬롯 및 변경 관련 로직
	void EquipItemFromCurrentQuickSlot(int32 QuickSlotIndex);

	// 퀵슬롯 아이템들 (타입은 아이템 구조에 따라 UObject*, AItemBase*, UItemData* 등)
	//TArray<UObject*> QuickSlots;

	// 현재 장착된 아이템
	UObject* HeldItem = nullptr;

	//TODO: 아이템 클래스 들어오면 반환 값 바꾸기
	void GetHeldItem();
	void EquipItem(UObject* Item);
	void UnequipCurrentItem();

	//퀵슬롯 칸 최대 칸 수
	int32 MaxQuickSlotIndex = 3;
	//현재 퀵슬롯 인덱스
	UPROPERTY(Replicated)
	int32 CurrentQuickSlotIndex = 0;

	UFUNCTION(Server, Reliable)
	void Server_SetQuickSlotIndex(int32 NewIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipItemFromQuickSlot(int32 Index);

	int32 GetCurrentQuickSlotIndex();
	void SetCurrentQuickSlotIndex(int32 NewIndex);

	UFUNCTION()
	void HandleInventoryUpdated();


	//Character Movement
public:
	bool CheckHardLandState();

	EPlayerState CheckPlayerCurrentState();

	UFUNCTION(Client, Reliable)
	void Client_SetMovementSetting();
	void Client_SetMovementSetting_Implementation();

	void SetMovementSetting();
	TArray<float> CalculateMovementSpeedWithWeigth();
	void ResetMovementSetting();



	//달리기 관련 로직
	float GetPlayerMovementSpeed();

	void ConsumeStamina();
	void TickStaminaDrain();
	void StartStaminaDrain();
	void StopStaminaDrain();
	void StartStaminaRecovery();
	void StopStaminaRecovery();
	void StartStaminaRecoverAfterDelay();
	void StartStaminaRecoverAfterDelayOnJump();
	void StopStaminaRecoverAfterDelay();
	void TickStaminaRecovery();
	bool HasStamina() const;
	bool IsStaminaFull() const;
private:
	FTimerHandle StaminaDrainHandle;
	FTimerHandle StaminaRecoveryHandle;
	FTimerHandle StaminaRecoveryDelayHandle;

	// 인벤토리 아이템 관련 변수 및 함수
public:
	UPROPERTY()
	FGameplayTagContainer OwnedTags;

	UChildActorComponent* ChildActorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UItemSpawnerComponent* ItemSpawner;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Equipment")
	UStaticMeshComponent* BackpackMeshComponent;

public:
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UToolbarInventoryComponent* GetToolbarInventoryComponent() const;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	UBackpackInventoryComponent* GetBackpackInventoryComponent() const;
private:
	UPROPERTY(Replicated)
	FGameplayTagContainer EquippedTags;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool IsEquipped() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquipped(bool bEquip);

public:
	// 임의적으로 만든 캐릭터의 상호작용 함수
	bool TryPickupItem(AItemBase* HitItem);
	UFUNCTION(Server, Reliable)
	void Server_TryPickupItem(AItemBase* HitItem);
	void Server_TryPickupItem_Implementation(AItemBase* ItemToPickup);

protected:
	/** 실제 아이템 습득 로직 (서버에서만 실행) */
	bool TryPickupItem_Internal(AItemBase* ItemActor);

public:
	UFUNCTION(Server, Reliable)
	void Server_UnequipCurrentItem();
	void Server_UnequipCurrentItem_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_EquipItemFromCurrentQuickSlot(int32 QuickSlotIndex);
	void Server_EquipItemFromCurrentQuickSlot_Implementation(int32 QuickSlotIndex);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UseEquippedItem();
	UFUNCTION(Server, Reliable)
	void Server_UseEquippedItem();
	void Server_UseEquippedItem_Implementation();

public:
	/** 인벤토리 UI를 토글합니다 */
	UFUNCTION(BlueprintCallable, Category = "Character|UI")
	void ToggleInventory();

	/** 인벤토리가 현재 열려있는지 확인 */
	UFUNCTION(BlueprintCallable, Category = "Character|UI")
	bool IsInventoryOpen() const;

private:
	/** 인벤토리 UI가 열려있는지 추적 */
	UPROPERTY(Replicated)
	bool bInventoryOpen = false;

public:
	/** 현재 장착된 아이템 드랍 */
	UFUNCTION(BlueprintCallable, Category = "Character|Inventory")
	void DropCurrentItem();

	/** 특정 슬롯 아이템 드랍 */
	UFUNCTION(BlueprintCallable, Category = "Character|Inventory")
	void DropItemAtSlot(int32 SlotIndex, int32 Quantity = 1);

public:
	//-----------------------------------------------------
	// 가방 관리 (간소화)
	//-----------------------------------------------------

	/** 가방 장착 (데이터 복사 방식) */
	UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
	bool EquipBackpack(FName BackpackItemRowName, const TArray<FBaseItemSlotData>& BackpackData, int32 MaxSlots);

	/** 가방 해제 (데이터 반환) */
	UFUNCTION(BlueprintCallable, Category = "Character|Equipment")
	TArray<FBaseItemSlotData> UnequipBackpack();

	/** 가방이 장착되어 있는지 확인 */
	UFUNCTION(BlueprintPure, Category = "Character|Equipment")
	bool HasBackpackEquipped() const;

private:
	/** 가방 메시 설정 */
	void SetBackpackMesh(UStaticMesh* BackpackMesh);

public:
	/** 인벤토리 무게 변경 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Character|Weight")
	void OnInventoryWeightChanged(float WeightDifference);

	/** 총 무게 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Character|Weight")
	float GetTotalCarryingWeight() const;

protected:
	/** 현재 총 무게 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character|Weight")
	float CurrentTotalWeight = 0.0f;
};
