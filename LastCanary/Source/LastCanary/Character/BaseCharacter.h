#pragma once

#include "CoreMinimal.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsCharacter.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "Interface/GimmickDebuffInterface.h"
#include "GameplayTagAssetInterface.h"
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
struct FBackpackSlotData;
class UItemSpawnerComponent;
class UPostProcessComponent;
class AResourceNode;

UCLASS()
class LASTCANARY_API ABaseCharacter : public AAlsCharacter , public IGimmickDebuffInterface , public IGameplayTagAssetInterface
{
	GENERATED_BODY()

	//Character Mesh and Component
public:
	/*1인칭 전용 메시 (자신만 보이는)*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	UStaticMeshComponent* OverlayStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* OverlaySkeletalMesh;

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


	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> ThirdPersonArrow;

	// 관전용 스프링암
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpectatorSpringArm;

	// 관전용 카메라
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* SpectatorCamera;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* HeadMesh;

	UPROPERTY(VisibleAnywhere)
	UPostProcessComponent* CustomPostProcessComponent;

	float GetBrightness();
	void SetBrightness(float Value);

	virtual void Tick(float DeltaSeconds);

	float WallClipAimOffsetPitch;
	float MaxWallClipPitch = 90.0f;
	float CapsuleWallRatio = 0.0f;
	void UpdateGunWallClipOffset(float DeltaTime);

	int LerpCount = 0;
	// Camera 이동 관련
	FTimerHandle CameraLerpTimerHandle;
	float LerpAlpha = 0.0f;
	FVector InitialCameraOffset;
	FVector TargetCameraOffset;
	bool bIsAiming = false;
	UPROPERTY()
	FVector DefaultSpringArmRelativeLocation;

	UPROPERTY()
	FName SpringArmAttachSocketName = NAME_None;

	FVector CurrentCameraLocation;
	FVector TargetCameraLocation;
	// 오프셋 보간 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	bool bShouldLerpCamera = false;

	void StartAiming();

	void StopAiming();

	UPROPERTY()
	bool bIsSmoothTransitioning = false;
	UPROPERTY()
	bool bIsTransitioning = false;
	UPROPERTY()
	FRotator TargetCameraRotation;

	UPROPERTY()
	float CameraTransitionSpeed = 15.0f;

	//Character Default Settings
protected:
	/*Character Default Settings*/
	ABaseCharacter();
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
	virtual void NotifyControllerChanged() override;
	virtual void BeginPlay() override;


	// Camera Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float MaxPitchAngle{ 60.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Meta = (ClampMin = -80, ClampMax = 0, ForceUnits = "deg"))
	float MinPitchAngle{ -60.0f };

	virtual void CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo) override;

	void AttachCameraToRifle();
	void AttachCameraToCharacter();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CurrentRifleMesh;

	FTimerHandle MoveTimerHandle;
	FVector StartLocation;
	//FVector TargetLocation;
	FName TargetSocketName = FName("");
	float InterpSpeed = 15.0f;
	float SnapTolerance = 1.0f;

	bool bIsFPSCamera = true;
	bool bDesiredADS = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothCameraSpeed = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float SmoothCameraTimeThreshold = 0.5f;

	float SmoothCameraCurrentTime = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.f;

	bool bADS = false; // 현재 정조준 상태인가?

	bool bIsCloseToWall = false;
	bool bIsSprinting = false;
	// ABaseCharacter.h

	FVector LastCameraLocation;
	FRotator LastCameraRotation;

	FVector CameraLocationTarget;
	FRotator CameraRotationTarget;

	/*AIAIAIAIAAI*/
public:
	virtual void NotifyNoiseToAI(FVector Velocity) override;
	virtual void NotifyNoiseToAI(float LandVelocity) override;
	void MakeNoiseSoundToAI(float Force);
	void MakeNoiseSoundToBoss(float Force);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AISettings", meta = (ClampMin = "1.0", ClampMax = "10000.0"))
	float SoundLoudnessDivider = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AISettings", meta = (ClampMin = "1.0", ClampMax = "10000.0"))
	float MaxSoundRange = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AISettings")
	FName AISoundCheckTag = "CaveMonster";


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_HelmBoots;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_HelmBoots_Glassess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_Glassess;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_Teeth;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_Eyelash;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_CORNEA;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* DefaultHeadMaterial_EYEBALL;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	UMaterialInterface* TransparentHeadMaterial;

	// MyCharacter.h

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TArray<USkeletalMesh*> SkeletalMeshOptions;


public:
	void SetCameraMode(bool bIsFirstPersonView);




	void SwapHeadMaterialTransparent(bool bUseTransparent);
public:

	// Header 파일에 추가할 변수들
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float BaseRecoilPitch = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float BaseRecoilYaw = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float RecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	float RecoilRecoverySpeed = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	int32 MaxConsecutiveShots = 10;

	// 현재 연사 상태
	int32 CurrentShotCount = 0;
	FVector2D AccumulatedRecoil = FVector2D::ZeroVector;
	FVector2D TargetRecoil = FVector2D::ZeroVector;
	FTimerHandle RecoilRecoveryTimer;
	FTimerHandle ShotResetTimer;

	// 반동 패턴 (선택사항 - CS:GO 스타일)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil")
	TArray<FVector2D> RecoilPattern;

	// 반동 상태 변수들

	//void ApplyRecoilStep();
	//void CameraShake(float Vertical, float Horizontal);
	void ApplyRecoil(float Vertical, float Horizontal);
	void RecoverFromRecoil();
	void ApplySmoothRecoil(float Vertical, float Horizontal);
	void ApplySmoothRecoilStep();
	void ResetShotCounter();
	void UpdateRecoil();
	bool HasActiveRecoil() const;
	void ReduceRecoil(float ReductionFactor = 0.5f);
	void ResetRecoilYaw();
	void ResetRecoilPitch();
	void ResetRecoil();
	FVector2D GetCurrentRecoil() const;
	// Character Input Handle Function

public:
	/*Function called by the controller*/
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity);
	virtual void Handle_Look(const FInputActionValue& ActionValue);
	virtual void Handle_Move(const FInputActionValue& ActionValue);
	virtual void Handle_Sprint(const FInputActionValue& ActionValue);
	virtual void Handle_Walk(const FInputActionValue& ActionValue);
	virtual void Handle_Crouch(const FInputActionValue& ActionValue);
	virtual void Handle_Jump(const FInputActionValue& ActionValue);
	virtual void Handle_Strafe(const FInputActionValue& ActionValue);
	virtual void Handle_Aim(const FInputActionValue& ActionValue);
	virtual void Handle_Interact(const FInputActionValue& ActionValue);
	virtual void Handle_ViewMode();
	virtual void Handle_Reload();
	virtual void Handle_VoiceChatting(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void UpdateVoiceChannelBySoectateState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void StartVoiceChat();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void CancelVoiceChat();




	void EscapeThroughGate();

	//Character State

public:


	bool bIsScoped = false;
	bool bIsPossessed;
	bool bIsReloading = false;
	bool bIsClose = false;
	bool bIsUsingItem = false;
	void SetPossess(bool IsPossessed);
	bool bRecoveringFromRecoil = false;

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

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RefreshOverlayObject(int index);
	void Multicast_RefreshOverlayObject_Implementation(int index);

	bool bIsSpawnDrone = false;

	UFUNCTION(Server, Reliable)
	void Server_UnPossessDrone();
	void Server_UnPossessDrone_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UnPossessDrone();
	void NetMulticast_UnPossessDrone_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> DefaultAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> RifleAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> PistolAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> TorchAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	TSubclassOf<UAnimInstance> BinocularsAnimationClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	USkeletalMesh* SKM_Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	USkeletalMesh* SKM_Pistol;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UStaticMesh* SM_Torch;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UStaticMesh* RCController;

public:
	//애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* InteractMontageOnUpperObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* InteractMontageOnUnderObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* KickMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* PressButtonMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* OpeningValveMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* PickAxeMontage;


	FTimerHandle DroneTrackingTimerHandle;
	void StartTrackingDrone();
	void StopTrackingDrone();
	void UpdateRotationToDrone();

	class ABaseDrone* ControlledDrone;

	void InteractAfterPlayMontage(AActor* TargetActor);
	void OnInteractAnimationNotified();

	UFUNCTION(Server, Unreliable)
	void Server_PlayMontage(UAnimMontage* MontageToPlay);
	void Server_PlayMontage_Implementation(UAnimMontage* MontageToPlay);

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_PlayMontage(UAnimMontage* MontageToPlay);
	void Multicast_PlayMontage_Implementation(UAnimMontage* MontageToPlay);

	UPROPERTY()
	UAnimMontage* CurrentInteractMontage;

	void CancelInteraction();

	UFUNCTION(Server, Unreliable)
	void Server_CancelInteraction();
	void Server_CancelInteraction_Implementation();


	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_CancelInteraction();
	void Multicast_CancelInteraction_Implementation();

	bool bIsPlayingInteractionMontage = false;



	void UseItemAfterPlayMontage(AItemBase* EquippedItem);
	void UseItemAnimationNotified();

	UPROPERTY()
	UAnimMontage* CurrentUseItemMontage;

	UPROPERTY()
	AItemBase* CurrentUsingItem;

	void CancelUseItem();

	UFUNCTION(Server, Unreliable)
	void Server_CancelUseItem();
	void Server_CancelUseItem_Implementation();

	UFUNCTION(NetMulticast, Unreliable)
	void Multicast_CancelUseItem();
	void Multicast_CancelUseItem_Implementation();

	bool bIsPlayingUseItemMontage = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* UsingBandageMontage;



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



	UPROPERTY()
	AActor* InteractTargetActor;
	//Player Take Damage
public:
	/*Player Damage, Death*/
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void HandlePlayerDeath();

	void NotifyPlayerDeathToGameState();

	UFUNCTION(Client, Reliable)
	void Client_HandlePlayerVoiceChattingState();
	void Client_HandlePlayerVoiceChattingState_Implementation();


	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerInGameStateOnDie();
	void Multicast_SetPlayerInGameStateOnDie_Implementation();

	virtual void GetFallDamage(float Velocity) override;

	float CalculateTakeDamage(float DamageAmount);
	float CalculateFallDamage(float Velocity);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerInGameStateOnEscapeGate();
	void Multicast_SetPlayerInGameStateOnEscapeGate_Implementation();


	void RequestReload(class AGunBase* Gun);
	void StartReload();
	void GunReloadAnimationNotified();


	UFUNCTION(Server, Reliable)
	void Server_PlayReload();
	void Server_PlayReload_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayReload();
	void Multicast_PlayReload_Implementation();

	void StopReload();

	
	
	UFUNCTION(Server, Reliable)
	void Server_StopReload();
	void Server_StopReload_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopReload();
	void Multicast_StopReload_Implementation();


public:

	void PickupItem();

	// 퀵슬롯 아이템들 (타입은 아이템 구조에 따라 UObject*, AItemBase*, UItemData* 등)
	//TArray<UObject*> QuickSlots;

	// 현재 장착된 아이템
	UObject* HeldItem = nullptr;

	//TODO: 아이템 클래스 들어오면 반환 값 바꾸기
	void GetHeldItem();

	void UnequipCurrentItem();

	UFUNCTION(Server, Reliable)
	void Server_SetQuickSlotIndex(int32 NewIndex);
	void Server_SetQuickSlotIndex_Implementation(int32 NewIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EquipItemFromQuickSlot(int32 Index);
	void Multicast_EquipItemFromQuickSlot_Implementation(int32 Index);

	int32 GetCurrentQuickSlotIndex();
	void SetCurrentQuickSlotIndex(int32 NewIndex);
	void EquipItemFromCurrentQuickSlot(int32 QuickSlotIndex);

	UFUNCTION(Server, Reliable)
	void Server_EquipItemFromCurrentQuickSlot(int32 QuickSlotIndex);
	void Server_EquipItemFromCurrentQuickSlot_Implementation(int32 QuickSlotIndex);


	void StopCurrentPlayingMontage();

	UFUNCTION()
	void HandleInventoryUpdated();


	//Character Movement
public:
	bool CheckHardLandState();

	EPlayerInGameStatus CheckPlayerCurrentState();

	UFUNCTION(Client, Reliable)
	void Client_SetMovementSetting();
	void Client_SetMovementSetting_Implementation();

	void SetMovementSetting();
	TArray<float> CalculateMovementSpeedWithWeigth();
	void ResetMovementSetting();

	float FrontInput = 0.0f;

	//달리기 관련 로직
	float GetPlayerMovementSpeed() const;

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
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tags")
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

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UseEquippedItem(float ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_UseEquippedItem(float ActionValue);
	void Server_UseEquippedItem_Implementation(float ActionValue);

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

	/** 캐릭터 사망 시 모든 슬롯 아이템 드랍 */
	UFUNCTION(BlueprintCallable, Category = "Character|Inventory")
	void DropAllItemsOnDeath();

	//-----------------------------------------------------
	// 가방 관리 (간소화)
	//-----------------------------------------------------

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

public:
	/** 현재 장착된 총기의 발사 모드 전환 (단발 ↔ 연발) */
	UFUNCTION(BlueprintCallable, Category = "Character|Weapon")
	void ToggleFireMode();

	/** 스캐너를 위한 스텐실 설정 */
	void EnableStencilForAllMeshes(int32 StencilValue);

	//-----------------------------------------------------
	// 각종 자원 채집을 위한 상호작용
	//-----------------------------------------------------

	UFUNCTION(Server, Reliable)
	void Server_InteractWithResourceNode(AResourceNode* TargetNode);
	void Server_InteractWithResourceNode_Implementation(AResourceNode* TargetNode);

	// 체력 회복 관련 함수
	FTimerHandle HealingTimerHandle;
	int32 HealingTicksRemaining = 0;
	float HealingPerTick = 0.f;

	UFUNCTION()
	void StartHealing(float TotalHealAmount, float Duration);

	UFUNCTION()
	void HealStep();

	UFUNCTION()
	void StopHealing();

	// 디버프 인터페이스 
	virtual void ApplyMovementDebuff_Implementation(float SlowRate, float Duration) override;
	virtual void RemoveMovementDebuff_Implementation() override;

	virtual void GetOwnedGameplayTags(FGameplayTagContainer& TagContainer) const override;

	UPROPERTY(BlueprintReadWrite, Category = "Debuff")
	bool bIsMovementDebuffed = false;

	UPROPERTY(BlueprintReadWrite, Category = "Debuff")
	float DebuffSlowRate = 1.f;

	// 원래 속도 저장용
	float OriginalWalkSpeed = 0.f;
	float OriginalRunSpeed = 0.f;
	float OriginalSprintSpeed = 0.f;
	//-----------------------------------------------------
	// 보이스 채팅을 위한 함수들
	//-----------------------------------------------------
public:
	/** 워키토키 채널 추가 (블루프린트에서 구현) */
	UFUNCTION(BlueprintImplementableEvent)
	void AddWalkieTalkieChannel();

	/** 워키토키 채널 제거 (블루프린트에서 구현) */
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveWalkieTalkieChannel();

private:
	/** 현재 워키토키 채널이 활성화되어 있는지 */
	UPROPERTY(BlueprintReadOnly, Category = "WalkieTalkie", meta = (AllowPrivateAccess = "true"))
	bool bHasWalkieTalkieChannel = false;

public:
	/** 워키토키 채널 상태 확인 */
	UFUNCTION(BlueprintPure, Category = "WalkieTalkie")
	bool HasWalkieTalkieChannel() const { return bHasWalkieTalkieChannel; }

	/** 워키토키 채널 상태 설정 (C++에서 호출용) */
	UFUNCTION(BlueprintCallable, Category = "WalkieTalkie")
	void SetWalkieTalkieChannelStatus(bool bActive);

	/** 특정 클라이언트에서 워키토키 획득 시 채널 상태 업데이트 */
	UFUNCTION(Client, Reliable, Category = "WalkieTalkie")
	void Client_SetWalkieTalkieChannelStatus(bool bActive);
	void Client_SetWalkieTalkieChannelStatus_Implementation(bool bActive);
};
