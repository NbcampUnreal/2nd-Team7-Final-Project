// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/PlayerController/LCPlayerInputController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

void ALCPlayerInputController::BeginPlay()
{
	Super::BeginPlay();
	auto* InputSubsystem{ ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()) };
	if (IsValid(InputSubsystem))
	{
		FModifyContextOptions Options;
		Options.bNotifyUserSettings = true;

		InputSubsystem->AddMappingContext(InputMappingContext, 0, Options);

	}
	//InitInputComponent();


	DefaultMouseCursor = EMouseCursor::Default;
	CurrentMouseCursor = EMouseCursor::Default;
}

void ALCPlayerInputController::InitInputComponent()
{
	EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

	if (!IsValid(EnhancedInput))
	{
		return;
	}

	if (IsValid(EnhancedInput))
	{
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnLookMouse);
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnLookMouse);

		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnLook);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnLook);

		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnMove);

		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnSprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnSprint);

		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnWalk);
		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnWalk);

		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnCrouch);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnCrouch);

		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnJump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnJump);

		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnAim);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnAim);

		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnViewMode);

		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnInteract);
		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnInteract);

		EnhancedInput->BindAction(StrafeAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnStrafe);

		EnhancedInput->BindAction(ItemUseAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OnItemUse);
		EnhancedInput->BindAction(ItemUseAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_OnItemUse);

		EnhancedInput->BindAction(ThrowItemAction, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_OnItemThrow);

		EnhancedInput->BindAction(RifleReloadAction, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_Reload);

		EnhancedInput->BindAction(VoiceAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_VoiceChat);
		EnhancedInput->BindAction(VoiceAction, ETriggerEvent::Canceled, this, &ALCPlayerInputController::Input_VoiceChat);

		EnhancedInput->BindAction(ChangeShootingSettingAction, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_ChangeShootingSetting);

		EnhancedInput->BindAction(ChangeQuickSlotAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_ChangeQuickSlot);

		EnhancedInput->BindAction(SelectQuickSlot1Action, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_SelectQuickSlot1);
		EnhancedInput->BindAction(SelectQuickSlot2Action, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_SelectQuickSlot2);
		EnhancedInput->BindAction(SelectQuickSlot3Action, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_SelectQuickSlot3);
		EnhancedInput->BindAction(SelectQuickSlot4Action, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_SelectQuickSlot4);

		EnhancedInput->BindAction(OpenPauseMenuAction, ETriggerEvent::Triggered, this, &ALCPlayerInputController::Input_OpenPauseMenu);

		EnhancedInput->BindAction(ExitDroneAction, ETriggerEvent::Started, this, &ALCPlayerInputController::Input_DroneExit);

		EnhancedInput->BindAction(RoomUIAction, ETriggerEvent::Started, this, &ALCPlayerInputController::ToggleShowRoomWidget);
	}

	ApplyInputMappingContext(InputMappingContext);
}

void ALCPlayerInputController::Input_OnLookMouse(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnLook(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnMove(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnSprint(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnWalk(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnCrouch(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnJump(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnAim(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnViewMode(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnInteract(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnStrafe(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnItemUse(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OnItemThrow(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_VoiceChat(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_ChangeShootingSetting(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_Reload(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_ChangeQuickSlot(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_SelectQuickSlot1(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_SelectQuickSlot2(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_SelectQuickSlot3(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_SelectQuickSlot4(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_OpenPauseMenu(const FInputActionValue& ActionValue) { }
void ALCPlayerInputController::Input_DroneExit(const FInputActionValue& ActionValue){ }
void ALCPlayerInputController::ToggleShowRoomWidget(){ }

void ALCPlayerInputController::ApplyInputMappingContext(UInputMappingContext* IMC)
{
	UE_LOG(LogTemp, Warning, TEXT("Apply Input Mapping Context"));
	if (const auto* LocalPlayer = GetLocalPlayer())
	{
		if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			FModifyContextOptions Options;
			Options.bNotifyUserSettings = true;
			UE_LOG(LogTemp, Warning, TEXT("Added Input Mapping Context"));

			InputSubsystem->AddMappingContext(IMC, 0, Options);
		}
	}
}

void ALCPlayerInputController::OnUIClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("컨트롤러에서 좌클릭 로직 실행됨"));
	CurrentMouseCursor = EMouseCursor::Custom;
	
	// 여기서 게임 상태 변경, 사운드 재생 등 원하는 처리 가능
}

void ALCPlayerInputController::OnUIReleased()
{
	UE_LOG(LogTemp, Warning, TEXT("컨트롤러에서 좌클릭 떼는 로직 실행됨"));
	CurrentMouseCursor = EMouseCursor::Default;
}