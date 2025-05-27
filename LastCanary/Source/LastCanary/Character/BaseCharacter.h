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

class AItemBase;
class UToolbarInventoryComponent;
class UBackpackInventoryComponent;

UCLASS()
class LASTCANARY_API ABaseCharacter : public AAlsCharacter
{
	GENERATED_BODY()
public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	UStaticMeshComponent* OverlayStaticMesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "CharacterMesh")
	USkeletalMeshComponent* OverlaySkeletalMesh;

	/*IsCharacterPossess?*/
public:
	void SetPossess(bool IsPossessed);
private:
	bool bIsPossessed;

	/*Character Default Settings*/
public:
	ABaseCharacter();
	void GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const;
	virtual void NotifyControllerChanged() override;
protected:
	virtual void BeginPlay() override;


	// Camera
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Settings|Camera")
	TObjectPtr<UAlsCameraComponent> Camera;

protected:
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& ViewInfo) override;

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

	/*Character Action Function*/
public: //Functions to process controller input.
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue);

	virtual void Handle_Look(const FInputActionValue& ActionValue);

	virtual void Handle_Move(const FInputActionValue& ActionValue);

	virtual void Handle_Sprint(const FInputActionValue& ActionValue);

	virtual void Handle_Walk(const FInputActionValue& ActionValue);

	virtual void Handle_Crouch();

	virtual void Handle_Jump(const FInputActionValue& ActionValue);

	virtual void Handle_Aim(const FInputActionValue& ActionValue);

	virtual void Handle_ViewMode();

	virtual void Handle_Interact(AActor* HitActor);

	virtual void Handle_Strafe(const FInputActionValue& ActionValue);

public: //Interact Function
	void PickupItem();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	UBoxComponent* InteractDetectionBox;

	// 현재 바라보고 있는 상호작용 가능한 액터
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Interact")
	AActor* CurrentFocusedActor;

	UFUNCTION()
	void OnInteractBoxBeginOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractBoxEndOverlap(UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
	
	FTimerHandle OverlapCheckTimerHandle;
	void OverlapCheckFunction();

public:
	// 아이템 퀵슬롯 및 변경 관련 로직
	void EquipItemFromCurrentQuickSlot(int32 QuickSlotIndex);

	// 퀵슬롯 아이템들 (타입은 아이템 구조에 따라 UObject*, AItemBase*, UItemData* 등)
	TArray<UObject*> QuickSlots;

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


	/*Player Damage, Death*/
public:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void HandlePlayerDeath();
	virtual float GetFallDamage(float Amount) override;
	
	//Character Movement
public:
	bool CheckHardLandState();

	EPlayerState CheckPlayerCurrentState();

	void SetMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed);

	void ResetMovementSetting();

public:
	//Only Test	
	void TestEquipFunction(int32 NewIndex);

	//test Function
	void Handle_SprintOnPlayerState(const FInputActionValue& ActionValue, float multiplier);

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



	// 게임 플래이 태그를 통해 플레이어 확인용으로 사용
public:
	UPROPERTY()
	FGameplayTagContainer OwnedTags;

	UChildActorComponent* ChildActorComponent;

	// 인벤토리 컴포넌트
private:
	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UToolbarInventoryComponent* ToolbarInventoryComponent;

	UPROPERTY(VisibleAnywhere, Category = "Inventory")
	UBackpackInventoryComponent* BackpackInventoryComponent;

public:
	// TODO : 인벤토리 컴포넌트와 상호작용하는 함수 추가 필요
	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetBackpackInventoryComponent(UBackpackInventoryComponent* BackpackInvenComp, bool bEquip);

	// 게임 플레이 태그통한 장비 장비/해제 상태 부여 및 확인을 위해 추가한 부분입니다.
private:
	UPROPERTY(Replicated)
	FGameplayTagContainer EquippedTags;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool IsEquipped() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquipped(bool bEquip);
	
	// 임의적으로 만든 캐릭터의 상호작용 함수
	bool TryPickupItem(AItemBase* HitItem);
	UFUNCTION(Server, Reliable)
	void Server_TryPickupItem(AItemBase* HitItem);
	void Server_TryPickupItem_Implementation(AItemBase* ItemToPickup);

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

	// 임시 인벤토리 확인용
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
};
