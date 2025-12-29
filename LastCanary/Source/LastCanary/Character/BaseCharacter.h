#pragma once

#include "CoreMinimal.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsCharacter.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "Interface/GimmickDebuffInterface.h"
#include "Interface/NoiseEmitterInterface.h"
#include "SaveGame/LCLocalPlayerSaveGame.h"
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
struct FBaseItemSlotData;
struct FBackpackSlotData;
class UItemSpawnerComponent;
class UPostProcessComponent;
class AResourceNode;
class UWidgetComponent;
class UPlayerNameWidget;
class UCustomizationMeshMap;
struct FCharacterCustomizationData;
class UCharacterBaseComponent;
class UCharacterHealthComponent;
class UCharacterStaminaComponent;
class UCharacterAnimationComponent;
class UCharacterInteractionComponent;
class UCharacterFootstepNoiseComponent;
class UCharacterCameraControlComponent;
class UCharacterDisplayComponent;
class UCharacterAttackComponent;
class UCameraRecoilComponent;
class UCharacterInputComponent;
class UCharacterSpeedControlComponent;
class UCharacterSoundComponent;
class UCharacterSanityComponent;
class UCharacterADSComponent;
class UCharacterWeaponClippingComponent;

UENUM(BlueprintType)
enum class EAnimationType : uint8
{
	None UMETA(DisplayName = "None"),
	UseItem UMETA(DisplayName = "아이템 사용"),
	Interaction UMETA(DisplayName = "상호작용")
	// 필요한 상태 더 추가
};

UCLASS()
class LASTCANARY_API ABaseCharacter : public AAlsCharacter, public IGimmickDebuffInterface, public IGameplayTagAssetInterface, public INoiseEmitterInterface
{
	GENERATED_BODY()

protected:
	/*Character Default Settings*/
	ABaseCharacter();
	void InitializeComponents();
	void InitializeDefaultComponents();
	void InitializeExtraComponents();
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds);
	virtual void CalcCamera(const float DeltaTime, FMinimalViewInfo& ViewInfo) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void NotifyControllerChanged() override;
	
	void ApplyNetworkSmoothSettings(float InNetUpdateFrequency, float InMinNetUpdateFrequency, float InNetCullDistance, ENetworkSmoothingMode InSmoothingMode, float InDeltaTime);
	
	
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Camera")
	FVector EyeOffsetLocal;
	// BaseCharacter.h

public:
	virtual FVector GetPawnViewLocation() const override;
	FVector GetDesiredCameraOffset() const;

	bool IsADS() const;

	FTransform GetADSCameraTransform() const;


public:
	//* Character State Flag *//
	//bool bCanMove = false;

	//Character Mesh and Component
#pragma region 컴포넌트
public:

	//* 3인칭 카메라 (개발용) *//
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> TPSCamera;

	//* 1인칭 카메라 *// //메인
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<USceneComponent> CameraRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> FPSCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<UStaticMeshComponent> OverlayStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> OverlaySkeletalMesh;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UArrowComponent> ThirdPersonArrow;

	// 캐릭터 인벤토리 컴포넌트
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	TObjectPtr<UToolbarInventoryComponent> ToolbarInventoryComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterHealthComponent> HealthComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterStaminaComponent> StaminaComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterAnimationComponent> AnimationComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Component")
	TObjectPtr<UCharacterInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterFootstepNoiseComponent> FootstepNoiseComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterCameraControlComponent> CameraControlComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterDisplayComponent> DisplayComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterAttackComponent> AttackComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCameraRecoilComponent> RecoilComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterSpeedControlComponent> SpeedControlComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterSoundComponent> SoundPlayComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterSanityComponent> SanityComponent;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterADSComponent> ADSComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UCharacterWeaponClippingComponent> WeaponClippingComponent;
#pragma endregion

	UPROPERTY()
	TArray<UCharacterBaseComponent*> ManagedComponents;

	AActor* CurrentFocusedActor;

	// 컴포넌트 준비 완료 신호
	void NotifyComponentReady(UCharacterBaseComponent* Component);
private:
	UPROPERTY()
	int32 ComponentsReadyCount = 0;

	UPROPERTY()
	int32 TotalComponentCount = 0;

	UPROPERTY()
	bool bAllComponentsReady = false;

	// 준비 완료 후 실행
	void InitializeCharacter();

#pragma region 메쉬 컴포넌트
public:
	UFUNCTION()
	virtual float GetCurrentNoiseLevel() const override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomHeadMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomGloveMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomJacketMesh_OwnerNoSee;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomJacketMesh_OwnerSee;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomPantsMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomBeltsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomHelmetMesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomArmorMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> CustomBootsMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMeshComponent> BackpackMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	TObjectPtr<USkeletalMesh> BackpackSkeletalMesh;

public:
	TObjectPtr<USkeletalMeshComponent> GetHeadMesh() const { return CustomHeadMesh; }
	TObjectPtr<USkeletalMeshComponent> GetGloveMesh() const { return CustomGloveMesh; }
	TObjectPtr<USkeletalMeshComponent> GetJacketMesh_OwnerNoSee() const { return CustomJacketMesh_OwnerNoSee; }
	TObjectPtr<USkeletalMeshComponent> GetJacketMesh_OwnerSee() const { return CustomJacketMesh_OwnerSee; }
	TObjectPtr<USkeletalMeshComponent> GetPantsMesh() const { return CustomPantsMesh; }
	TObjectPtr<USkeletalMeshComponent> GetBeltsMesh() const { return CustomBeltsMesh; }
	TObjectPtr<USkeletalMeshComponent> GetHelmetMesh() const { return CustomHelmetMesh; }
	TObjectPtr<USkeletalMeshComponent> GetArmorMesh() const { return CustomArmorMesh; }
	TObjectPtr<USkeletalMeshComponent> GetBootsMesh() const { return CustomBootsMesh; }
	TObjectPtr<USkeletalMeshComponent> GetBackpackMesh() const { return BackpackMesh; }

	TObjectPtr<USkeletalMesh> GetBackpackSkeletalMesh() const { return BackpackSkeletalMesh; }
#pragma endregion

#pragma region 커스터마이징 관련
	void SetCharacterPoseSynchronization();
	
public:
	//* Character Customizing *//
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Customization")
	TObjectPtr<UCustomizationMeshMap> CharacterMeshMap;

	FCharacterCustomizationData CharacterCustomizationData;
	FCharacterCustomizationData GetCustomizationData() const { return CharacterCustomizationData; }
	void SetCustomizationData(FCharacterCustomizationData data) { CharacterCustomizationData = data; }

	void SetPartMesh(USkeletalMeshComponent* Component, USkeletalMesh* LoadedMesh);
	void SetPartMaterial(USkeletalMeshComponent* Component, int32 MaterialIndex, UMaterialInterface* Material);

	UFUNCTION(Server, Reliable)
	void Server_UpdateCustomizationData();  // 이건 안쓰는듯?
	void Server_UpdateCustomizationData_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_SetCustomizationData(const FCharacterCustomizationData& CustomizingData);
	void Server_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetCustomizationData(const FCharacterCustomizationData& CustomizingData);
	void Multicast_SetCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData);

	UFUNCTION(Server, Reliable)
	void Server_ApplyCustomizationData(const FCharacterCustomizationData& CustomizingData);
	void Server_ApplyCustomizationData_Implementation(const FCharacterCustomizationData& CustomizingData);

	void ApplyCustomizationToAllPlayers(const FCharacterCustomizationData CustomizationData);

	void ApplyCustomization(const FCharacterCustomizationData CustomizationData);

	void InitializeCustomization();
	void LoadAndApplyCustomization();
	void LoadCustomizationSettings();
	void SaveCustomizationDataToPlayerState(const FCharacterCustomizationData& CustomizingData);

#pragma endregion


#pragma region 캐릭터 입력 관련

private:
	bool bEnableInput = true;
public:
	bool IsInputEnabled() const { return bEnableInput; }
	void SetInputEnabled(bool bEnabled) { bEnableInput = bEnabled; }

public:
	bool CheckCondition_LookMouse();
	bool CheckCondition_Move();
	bool CheckCondition_Sprint();
	bool CheckCondition_Walk();
	bool CheckCondition_Crouch();
	bool CheckCondition_Jump();
	bool CheckCondition_Aim();
	bool CheckCondition_Interact();
	bool CheckCondition_ViewMode();
	bool CheckCondition_Reload();
	bool CheckCondition_VoiceChatting();

	bool Check_PlayerController();
	bool Check_PlayerState();
	bool Check_InputEnabled();

	bool Check_DefaultCondition();

	/*Function called by the controller*/
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue, float Sensivity, float ZoomSensivity);
	virtual void Handle_Move(const FInputActionValue& ActionValue);
	virtual void Handle_Sprint(const FInputActionValue& ActionValue);
	virtual void Handle_Walk(const FInputActionValue& ActionValue);
	virtual void Handle_Crouch(const FInputActionValue& ActionValue);
	virtual void Handle_Jump(const FInputActionValue& ActionValue);
	virtual void Handle_Aim(const FInputActionValue& ActionValue);
	virtual void Handle_Interact(const FInputActionValue& ActionValue);
	virtual void Handle_ViewMode();
	virtual void Handle_Reload();
	virtual void Handle_VoiceChatting(const FInputActionValue& ActionValue);
	virtual void Handle_Attack(const FInputActionValue& ActionValue);
	virtual void Handle_Emote(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void UpdateVoiceChannelBySoectateState();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void StartVoiceChat();

	UFUNCTION(BlueprintImplementableEvent, Category = "Voice")
	void CancelVoiceChat();

#pragma endregion


	bool bIsPlayerStateReady() const;



public:
	void SetBrightness(float Value);

	bool bPossessedCheck = false; // 빙의 상태인지
	bool bIsAiming = false; // 줌 상태인지

	UPROPERTY()
	bool bIsSmoothTransitioning = false;
	UPROPERTY()
	bool bIsTransitioning = false;

	int32 ApplyWheelSelection(int32 Index);



#pragma region 초기 캐릭터 세팅
	void InitializePlayerLocalSettings();

	FTimerHandle RetryInitializeCustomizingHandle;
	void InitializePlayerCustomizing();

	UFUNCTION(Server, Reliable)
	void Server_ClientLogin();
	void Server_ClientLogin_Implementation();

public:
	void CheckPlayerCharacterIsReadyToGameMode();
#pragma endregion

	// Camera Settings
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Meta = (ClampMin = 0, ClampMax = 90, ForceUnits = "deg"))
	float MaxPitchAngle{ 60.0f };
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Camera", Meta = (ClampMin = -80, ClampMax = 0, ForceUnits = "deg"))
	float MinPitchAngle{ -60.0f };

	void ResetCameraLocationToDefault();

public:
	AItemBase* GetCurrentItem() const;

	UFUNCTION(BlueprintCallable)
	AGunBase* GetCurrentGunItem() const;

	UFUNCTION(BlueprintCallable)
	USkeletalMeshComponent* GetCurrentGunItemSkeletalMesh() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bHasGunOnHand = false;
	UFUNCTION(BlueprintCallable)
	void SetHasGunOnHand(bool _bhasgun) {bHasGunOnHand = _bhasgun;}
	UFUNCTION(BlueprintCallable)
	bool GetHasGunOnHand() {return bHasGunOnHand;}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* CurrentRifleMesh;

	bool bIsFPSCamera = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float FieldOfView = 90.f;


public:
	bool bIsCloseToWall = false;
	void SetIsCloseToWall(bool _bIsCloseToWall);
	bool GetIsCloseToWall();

	bool bIsSprinting = false;

#pragma region AI관련 함수

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

#pragma endregion



#pragma region 머티리얼 설정 관련
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

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
	TArray<USkeletalMesh*> SkeletalMeshOptions;

#pragma endregion


public:
	UFUNCTION(BlueprintCallable)
	void SetCameraMode(bool bIsFirstPersonView);

	UFUNCTION(BlueprintCallable)
	void SetCameraEmoteMode(bool bIsFirstPersonView);

	bool EmoteMode = false;

	void SwapHeadMaterialTransparent(bool bUseTransparent);

	void ApplySmoothRecoil(float Vertical, float Horizontal);

	void EscapeThroughGate();

	//Character State

public:
	void SetDamageEnabled(bool bEnabled);

public:
	bool bIsScoped = false;
	bool bIsPossessed;
	bool bIsReloading = false;
	bool bIsClose = false;
	bool bIsUsingItem = false;
	void SetPossess(bool IsPossessed);
	bool bRecoveringFromRecoil = false;


#pragma region 캐릭터 애니메이션 관련 오버레이 오브젝트

	//About Character Animation Montage and Animation Class
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	bool UseGunBoneforOverlayObjects;

	UFUNCTION(BlueprintCallable)
	void RefreshOverlayObject();

	UFUNCTION(BlueprintCallable)
	void AttachOverlayObject(UStaticMesh* NewStaticMesh, USkeletalMesh* NewSkeletalMesh, TSubclassOf<UAnimInstance> NewAnimationClass, FName SocketName, bool bUseLeftGunBone);

	UFUNCTION(BlueprintCallable)
	void RefreshOverlayLinkedAnimationLayer(FGameplayTag ItemTag);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_RefreshOverlayObject();
	void Multicast_RefreshOverlayObject_Implementation();

#pragma endregion



public:
	//애니메이션 몽타주
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* InteractMontageOnUpperObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* InteractMontageOnUnderObject;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* ReloadMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* PressButtonMontage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* OpeningValveMontage;




#pragma endregion 총기 재장전 관련
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

#pragma endregion


	void StopGunAutoFire();

#pragma region 드론 관련
	bool bIsSpawnDrone = false;

	UFUNCTION(Server, Reliable)
	void Server_UnPossessDrone();
	void Server_UnPossessDrone_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void NetMulticast_UnPossessDrone();
	void NetMulticast_UnPossessDrone_Implementation();


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Animation")
	UStaticMesh* RCController;

	FTimerHandle DroneTrackingTimerHandle;
	void StartTrackingDrone();
	void StopTrackingDrone();
	void UpdateRotationToDrone();

	class ABaseDrone* ControlledDrone;

#pragma endregion

	bool bIsPlayingAnimation = false;

	UFUNCTION(Client, Reliable)
	void Client_SetMiningState(bool NewValue);
	void Client_SetMiningState_Implementation(bool NewValue);

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

	UFUNCTION()
	void OnReloadFromNotify();

	UFUNCTION()
	void OnUseItemFromNotify();

	UFUNCTION()
	void HandleFocusChanged(AActor* NewFocus);

	bool bIsPlayingUseItemMontage = false;
	bool bIsMining = false;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animation")
	UAnimMontage* UsingBandageMontage;

public:

	UPROPERTY()
	AActor* InteractTargetActor;

#pragma region 데미지 및 체력 관련

	//Player Take Damage
public:

	/*Player Damage, Death*/
	UFUNCTION(BlueprintCallable)
	float TakeSanityDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual void GetFallDamage(float Velocity) override;

	float CalculateTakeDamage(float DamageAmount);
	float CalculateFallDamage(float Velocity);
	
	UFUNCTION()
	void HandlePlayerDeath();

	void NotifyPlayerDeathToGameState();

#pragma endregion



	// 감도 저장용
	float MouseSensitivityMultiplier = 1.0f;
	float MouseInvertMultiplier = 1.0f;

	

	UFUNCTION(Client, Reliable)
	void Client_HandlePlayerVoiceChattingState();
	void Client_HandlePlayerVoiceChattingState_Implementation();


	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerInGameStateOnDie();
	void Multicast_SetPlayerInGameStateOnDie_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetPlayerInGameStateOnEscapeGate();
	void Multicast_SetPlayerInGameStateOnEscapeGate_Implementation();


public:
	void UnequipCurrentItem();

	UFUNCTION(Server, Reliable)
	void Server_SetQuickSlotIndex(int32 NewIndex);
	void Server_SetQuickSlotIndex_Implementation(int32 NewIndex);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_ResetAnimationAndCamera(int32 Index);
	void Multicast_ResetAnimationAndCamera_Implementation(int32 Index);

	int32 GetCurrentQuickSlotIndex();
	void SetCurrentQuickSlotIndex(int32 NewIndex);

	void EquipItem(int32 Index);

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

	float SpeedMultiplier = 1.0f; // 0.0 ~ 1.0 범위
	float CalculateMovementSpeedMultiplier();
	float CalculateDebuffMultiplier();
	float MaxWeight = 200.0f;
	void ResetMovementSetting();

	float FrontInput = 0.0f;

	//달리기 관련 로직
	float GetPlayerMovementSpeed() const;

	UFUNCTION()
	void HandleStaminaConsumed();

	UFUNCTION()
	void HandleStaminaExhausted();

	UFUNCTION()
	void HandleStaminaThresholdReached();






	//////////********  내 코드 아님  ********//////////

	/** 가방 메시 설정 */
	void SetBackpackMesh(bool bIsEquipBackpack);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetBackpackMesh(bool bIsEquipBackpack);
	void Multicast_SetBackpackMesh_Implementation(bool bIsEquipBackpack);
	bool Updated = false;


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
	void UseEquippedItem(float ActionValue);
	UFUNCTION(Server, Reliable)
	void Server_UseEquippedItem(float ActionValue);
	void Server_UseEquippedItem_Implementation(float ActionValue);

	void UseItem(AItemBase* Item);
	void CancelUseItem(AItemBase* Item);

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

public:
	/** 현재 가방 메시 활성화 상태 추적 */
	UPROPERTY(Replicated)
	bool bBackpackMeshActive = false;

public:
	/** 인벤토리 무게 변경 시 호출 */
	UFUNCTION(BlueprintCallable, Category = "Character|Weight")
	void OnInventoryWeightChanged(float WeightDifference);

	/** 총 무게 가져오기 */
	UFUNCTION(BlueprintPure, Category = "Character|Weight")
	float GetTotalCarryingWeight() const;

	UFUNCTION(NetMulticast, Reliable)
	void Client_OnInventoryWeightChanged(float NewWeight);
	void Client_OnInventoryWeightChanged_Implementation(float NewWeight);

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

	UFUNCTION()
	void StartHealing(float TotalHealAmount, float Duration);

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

	// 워키토키 상태 확인 및 중지
	UFUNCTION(BlueprintCallable)
	void CheckAndStopWalkieTalkie();

	// 현재 장착된 워키토키가 사용 중인지 확인
	UFUNCTION(BlueprintPure)
	bool IsCurrentWalkieTalkieActive() const;

	/** 워키토키 채널 상태 설정 (C++에서 호출용) */
	UFUNCTION(BlueprintCallable, Category = "WalkieTalkie")
	void SetWalkieTalkieChannelStatus(bool bActive);

	/** 특정 클라이언트에서 워키토키 획득 시 채널 상태 업데이트 */
	UFUNCTION(Client, Reliable, Category = "WalkieTalkie")
	void Client_SetWalkieTalkieChannelStatus(bool bActive);
	void Client_SetWalkieTalkieChannelStatus_Implementation(bool bActive);

	virtual void OnRep_PlayerState() override;
};
