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

UCLASS()
class LASTCANARY_API ABaseCharacter : public AAlsCharacter
{
	GENERATED_BODY()
	/*IsCharacterPossess?*/
public:
	void SetPossess(bool IsPossessed);
private:
	bool bIsPossessed;

	void PossessedBy(AController* NewController)
	{
		Super::PossessedBy(NewController);

		if (HasAuthority())
		{
			UE_LOG(LogTemp, Warning, TEXT("서버에서 빙의됨! 컨트롤러: %s"), *NewController->GetName());
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("클라이언트에서 빙의됨! 컨트롤러: %s"), *NewController->GetName());
		}
		

		// 여기서 컨트롤러 빙의 직후 초기화나 상태 변경 처리 가능
	}

	/*Character Default Settings*/
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
public:
	//virtual void DisplayDebug(UCanvas* Canvas, const FDebugDisplayInfo& DisplayInfo, float& Unused, float& VerticalLocation) override;

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
	/*Equip Item*/
	void EquipItemFromCurrentQuickSlot(int QuickSlotIndex);

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
	int32 CurrentQuickSlotIndex = 0;
	int32 GetCurrentQuickSlotIndex();
	void SetCurrentQuickSlotIndex(int32 NewIndex);

public:
	UFUNCTION(BlueprintImplementableEvent)
	void TestEquipFunction(int32 NewIndex);
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

	//test Function
	void Handle_SprintOnPlayerState(const FInputActionValue& ActionValue, float multiplier);
};
