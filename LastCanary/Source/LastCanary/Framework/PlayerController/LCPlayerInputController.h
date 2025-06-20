// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "LCPlayerInputController.generated.h"

struct FInputActionValue;
class UEnhancedInputComponent;
class UInputMappingContext;
class UInputAction;

UCLASS()
class LASTCANARY_API ALCPlayerInputController : public APlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;
public:
	UEnhancedInputComponent* EnhancedInput;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> LookMouseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> WalkAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> CrouchAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> AimAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ViewModeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> InteractAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ItemUseAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ThrowItemAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> RifleReloadAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> VoiceAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ChangeShootingSettingAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> StrafeAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ChangeQuickSlotAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot1Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot2Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot3Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> SelectQuickSlot4Action;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> OpenPauseMenuAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|Character", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> ExitDroneAction;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "InputSettings|RoomPC", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> RoomUIAction;

public:
	virtual void InitInputComponent();

public:
	virtual void Input_OnLookMouse(const FInputActionValue& ActionValue);

	virtual void Input_OnLook(const FInputActionValue& ActionValue);

	virtual void Input_OnMove(const FInputActionValue& ActionValue);

	virtual void Input_OnSprint(const FInputActionValue& ActionValue);

	virtual void Input_OnWalk(const FInputActionValue& ActionValue);

	virtual void Input_OnCrouch(const FInputActionValue& ActionValue);

	virtual void Input_OnJump(const FInputActionValue& ActionValue);

	virtual void Input_OnAim(const FInputActionValue& ActionValue);

	virtual void Input_OnViewMode(const FInputActionValue& ActionValue);

	virtual void Input_OnInteract(const FInputActionValue& ActionValue);

	virtual void Input_OnStrafe(const FInputActionValue& ActionValue);

	virtual void Input_OnItemUse(const FInputActionValue& ActionValue);

	virtual void Input_OnItemThrow(const FInputActionValue& ActionValue);

	virtual void Input_VoiceChat(const FInputActionValue& ActionValue);

	virtual void Input_ChangeShootingSetting(const FInputActionValue& ActionValue);

	virtual void Input_Reload(const FInputActionValue& ActionValue);

	virtual void Input_ChangeQuickSlot(const FInputActionValue& ActionValue);

	virtual void Input_SelectQuickSlot1(const FInputActionValue& ActionValue);

	virtual void Input_SelectQuickSlot2(const FInputActionValue& ActionValue);

	virtual void Input_SelectQuickSlot3(const FInputActionValue& ActionValue);

	virtual void Input_SelectQuickSlot4(const FInputActionValue& ActionValue);

	virtual void Input_OpenPauseMenu(const FInputActionValue& ActionValue);

	virtual void Input_DroneExit(const FInputActionValue& ActionValue);
	
	UFUNCTION()
	virtual void ToggleShowRoomWidget();

	bool bIsShowRoomUI = false;

	void ApplyInputMappingContext(UInputMappingContext* IMC);
	

	void OnUIClicked(); // 클릭 전달받는 함수
	void OnUIReleased(); // 클릭 전달받는 함수
};
