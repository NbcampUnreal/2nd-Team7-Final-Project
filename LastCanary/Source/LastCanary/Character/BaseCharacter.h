// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../Plugins/ALS-Refactored-4.15/Source/ALS/Public/AlsCharacter.h"
//#include "../Plugins/ALS-Refactored-4.15/Source/ALSCamera/Public/AlsCameraComponent.h"
#include "BaseCharacter.generated.h"

struct FInputActionValue;
class UAlsCameraComponent;
class UInputMappingContext;
class UInputAction;

class UToolbarInventoryComponent;
class UBackpackInventoryComponent;

UCLASS()
class LASTCANARY_API ABaseCharacter : public AAlsCharacter
{
	GENERATED_BODY()
	

protected:
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly, Category = "Als Character Example")
	TObjectPtr<UAlsCameraComponent> Camera;


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


	// Input


public:
	virtual void Handle_LookMouse(const FInputActionValue& ActionValue);

	virtual void Handle_Look(const FInputActionValue& ActionValue);

	virtual void Handle_Move(const FInputActionValue& ActionValue);

	virtual void Handle_Sprint(const FInputActionValue& ActionValue);

	virtual void Handle_Walk();

	virtual void Handle_Crouch();

	virtual void Handle_Jump(const FInputActionValue& ActionValue);

	virtual void Handle_Aim(const FInputActionValue& ActionValue);

	virtual void Handle_Ragdoll();

	virtual void Handle_Roll();

	virtual void Handle_RotationMode();

	virtual void Handle_ViewMode();

	virtual void Handle_SwitchShoulder();


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
	UFUNCTION(BlueprintPure, Category = "Equipment")
	const FGameplayTagContainer& GetEquippedTags() const;

	UFUNCTION(BlueprintCallable, Category = "Equipment")
	void SetEquippedTags(const FGameplayTag& Tag, bool bEquip);
	
};
