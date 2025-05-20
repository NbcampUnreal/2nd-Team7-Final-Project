#pragma once

#include "CoreMinimal.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsCharacter.h"
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
	void SetPossess(bool IsPossessed);
private:
	bool bIsPossessed;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Als Character Example")
	TObjectPtr<UAlsCameraComponent> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Interact")
	UBoxComponent* InteractDetectionBox;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Als Character Example", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookUpMouseSensitivity{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Als Character Example", Meta = (ClampMin = 0, ForceUnits = "x"))
	float LookRightMouseSensitivity{ 1.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Als Character Example", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookUpRate{ 90.0f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings|Als Character Example", Meta = (ClampMin = 0, ForceUnits = "deg/s"))
	float LookRightRate{ 240.0f };

public:
	ABaseCharacter();

	virtual void NotifyControllerChanged() override;

	
	// Camera

protected:
	virtual void CalcCamera(float DeltaTime, FMinimalViewInfo& ViewInfo) override;

	//emigrated to controller//
	/*

	*/
public:
	//virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation) override;

	
public: //Functions to process controller input.
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue);

	virtual void Handle_Look(const FInputActionValue& ActionValue);

	virtual void Handle_Move(const FInputActionValue& ActionValue);

	virtual void Handle_Sprint(const FInputActionValue& ActionValue);

	virtual void Handle_Walk();

	virtual void Handle_Crouch();

	virtual void Handle_Jump(const FInputActionValue& ActionValue);

	virtual void Handle_Aim(const FInputActionValue& ActionValue);

	virtual void Handle_ViewMode();

	virtual void Handle_Interact(AActor* HitActor);

	virtual void Handle_Strafe(const FInputActionValue& ActionValue);

public: //Interact Function
	void PickupItem();


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

	void EquipItemFromCurrentQuickSlot(int QuickSlotIndex);

	// 퀵슬롯 아이템들 (타입은 아이템 구조에 따라 UObject*, AItemBase*, UItemData* 등)
	TArray<UObject*> QuickSlots;

	// 현재 장착된 아이템
	UObject* HeldItem = nullptr;

	void EquipItem(UObject* Item);
	void UnequipCurrentItem();


public:
	UFUNCTION(BlueprintImplementableEvent)
	void TestEquipFunction(int32 NewIndex);

public:
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	void HandlePlayerDeath();


	// 게임 플래이 태그를 통해 플레이어 확인용으로 사용
public:
	UPROPERTY()
	FGameplayTagContainer OwnedTags;

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
	UPROPERTY()
	FGameplayTagContainer EquippedTags;

public:
	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool IsEquipped() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquipped(bool bEquip);
	
	// 임의적으로 만든 캐릭터의 상호작용 함수
	bool TryPickupItem(AItemBase* HitItem);

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	bool UseEquippedItem();
};
