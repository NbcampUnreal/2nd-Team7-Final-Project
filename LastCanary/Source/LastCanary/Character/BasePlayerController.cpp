// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BasePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BaseCharacter.h"
#include "Kismet/GameplayStatics.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	
	CachedPawn = GetPawn();
}

void ABasePlayerController::ApplyInputMappingContext()
{
	UE_LOG(LogTemp, Warning, TEXT("Apply Input Mapping Context"));
	if (const auto* LocalPlayer = GetLocalPlayer())
	{
		if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			FModifyContextOptions Options;
			Options.bNotifyUserSettings = true;
			UE_LOG(LogTemp, Warning, TEXT("Added Input Mapping Context"));

			InputSubsystem->AddMappingContext(InputMappingContext, 0, Options);
		}
	}

	InitInputComponent();
}

void ABasePlayerController::RemoveInputMappingContext()
{
	UE_LOG(LogTemp, Warning, TEXT("Remove Input Mapping Context"));

	if (const auto* LocalPlayer = GetLocalPlayer())
	{
		if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer))
		{
			UE_LOG(LogTemp, Warning, TEXT("Deleted Input Mapping Context"));

			InputSubsystem->RemoveMappingContext(InputMappingContext);
		}
	}
}

void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("Player Possessed"));

	ApplyInputMappingContext();
}

void ABasePlayerController::OnUnPossess()
{
	RemoveInputMappingContext();
	UE_LOG(LogTemp, Warning, TEXT("Player Unpossessed"));

	Super::OnUnPossess();
}



void ABasePlayerController::InitInputComponent()
{
	EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);
	UE_LOG(LogTemp, Warning, TEXT("InputComponent class: %s"), *GetNameSafe(InputComponent));

	if (!IsValid(EnhancedInput))
	{
		UE_LOG(LogTemp, Warning, TEXT("There is no EnhancedInputComponent"));
		return;
	}

	if (IsValid(EnhancedInput))
	{
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnLookMouse);
		EnhancedInput->BindAction(LookMouseAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnLookMouse);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnLook);
		EnhancedInput->BindAction(LookAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnLook);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnMove);
		EnhancedInput->BindAction(MoveAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnMove);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnSprint);
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnSprint);
		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnWalk);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnCrouch);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnJump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnJump);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnAim);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnAim);
		EnhancedInput->BindAction(RagdollAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnRagdoll);
		EnhancedInput->BindAction(RollAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnRoll);
		EnhancedInput->BindAction(RotationModeAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnRotationMode);
		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnViewMode);
		EnhancedInput->BindAction(SwitchShoulderAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnSwitchShoulder);
	}
}

void ABasePlayerController::Input_OnLookMouse(const FInputActionValue& ActionValue)
{	
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_LookMouse(ActionValue);  // ABaseCharacter에 맞는 LookMouse 호출
		}
	}
	// 다른 타입의 Pawn이 있을 경우 추가적인 분기를 더할 수 있다.
}

void ABasePlayerController::Input_OnLook(const FInputActionValue& ActionValue)
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Look(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnMove(const FInputActionValue& ActionValue)
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Move(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnSprint(const FInputActionValue& ActionValue)
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Sprint(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnWalk()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Walk();
		}
	}
}

void ABasePlayerController::Input_OnCrouch()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter)) 
		{
			PlayerCharacter->Handle_Crouch();
		}
	}
}

void ABasePlayerController::Input_OnJump(const FInputActionValue& ActionValue)
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Jump(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnAim(const FInputActionValue& ActionValue)
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Aim(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnRagdoll()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Ragdoll();
		}
	}
}

void ABasePlayerController::Input_OnRoll()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Roll();
		}
	}
}

void ABasePlayerController::Input_OnRotationMode()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_RotationMode();
		}
	}
}

void ABasePlayerController::Input_OnViewMode()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_ViewMode();
		}
	}
}

// ReSharper disable once CppMemberFunctionMayBeConst
void ABasePlayerController::Input_OnSwitchShoulder()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_SwitchShoulder();
		}
	}
}