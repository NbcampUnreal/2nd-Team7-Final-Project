#include "Character/BasePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BaseCharacter.h"
#include "Item/Drone/BaseDrone.h"
#include "Kismet/GameplayStatics.h"
#include "BasePlayerState.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Controller Begin Play"));

	CachedPawn = GetPawn();

	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		PS->OnDamaged.AddDynamic(this, &ABasePlayerController::OnCharacterDamaged);
		PS->OnDied.AddDynamic(this, &ABasePlayerController::OnCharacterDied);
		PS->OnExhausted.AddDynamic(this, &ABasePlayerController::Complete_OnSprint);
	}
}

void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InitInputComponent();
}

void ABasePlayerController::OnCharacterDamaged()
{
	// HUD 갱신하거나 효과 표시 등
	UE_LOG(LogTemp, Warning, TEXT("Player took damage!"));

}

void ABasePlayerController::OnCharacterDied()
{
	// HUD 갱신하거나 효과 표시 등
	UE_LOG(LogTemp, Warning, TEXT("Player die!"));
	
	//드론에 빙의되어 있을 수도 있어서 이부분은 잘 처리를 해야....
	// 
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
			PlayerCharacter->HandlePlayerDeath(); //플레이어 사망처리
		}
	}
	else
	{

	}

	/*
	if(Drone)
	{
		Drone LookMouse
	}
	*/
	//HandlePlayerDeath();
}

APawn* ABasePlayerController::GetMyPawn()
{
	return CachedPawn;
}

void ABasePlayerController::SetMyPawn(APawn* NewPawn)
{
	CachedPawn = NewPawn;
}

void ABasePlayerController::ApplyInputMappingContext(UInputMappingContext* IMC)
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

void ABasePlayerController::RemoveInputMappingContext(UInputMappingContext* IMC)
{
	UE_LOG(LogTemp, Warning, TEXT("Remove Input Mapping Context"));

	if (const auto* LocalPlayer = GetLocalPlayer())
	{
		if (auto* InputSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(LocalPlayer)) 
		{
			UE_LOG(LogTemp, Warning, TEXT("Deleted Input Mapping Context"));

			InputSubsystem->RemoveMappingContext(IMC);
		}
	}
}

void ABasePlayerController::ChangeInputMappingContext(UInputMappingContext* IMC)
{
	CurrentIMC = IMC;
}

void ABasePlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	UE_LOG(LogTemp, Warning, TEXT("Player Possessed"));
	// Pawn의 타입에 따라 MappingContext를 자동 변경
	if (InPawn->IsA(ABaseCharacter::StaticClass()))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentIMC is Player"));
		CurrentIMC = InputMappingContext;
		Cast<ABaseCharacter>(InPawn)->SetPossess(true);
		SpanwedPlayerCharacter = Cast<ABaseCharacter>(InPawn);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentIMC is Drone"));
		CurrentIMC = DroneInputMappingContext;
		//SpawnedPlayerDrone = Cast<ABaseDrone>(InPawn);
	}
	ApplyInputMappingContext(CurrentIMC);
}

void ABasePlayerController::OnUnPossess()
{
	RemoveInputMappingContext(CurrentIMC);
	UE_LOG(LogTemp, Warning, TEXT("Player Unpossessed"));
	if (GetPawn()->IsA(ABaseCharacter::StaticClass()))
	{
		Cast<ABaseCharacter>(GetPawn())->SetPossess(false);
	}
	else
	{

	}
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

		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnViewMode);

		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnInteract);

		EnhancedInput->BindAction(StrafeAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnStrafe);
		EnhancedInput->BindAction(StrafeAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnStrafe);

		EnhancedInput->BindAction(ItemUseAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OnItemUse);
		
		EnhancedInput->BindAction(ChangeQuickSlotAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_ChangeQuickSlot);

		EnhancedInput->BindAction(SelectQuickSlot1Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot1);
		EnhancedInput->BindAction(SelectQuickSlot2Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot2);
		EnhancedInput->BindAction(SelectQuickSlot3Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot3);
		EnhancedInput->BindAction(SelectQuickSlot4Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot4);

		EnhancedInput->BindAction(OpenPauseMenuAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OpenPauseMenu);
	}

	ApplyInputMappingContext(InputMappingContext);
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
	/*
	if(Drone)
	{
		Drone LookMouse
	}
	*/
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
	/*
	if(Drone)
	{
		Drone MoveUp
		Drone MoveRight
	}
	
	*/
}


void ABasePlayerController::Input_OnSprint(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Sprint Triggered"));
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
			if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
			{
				UE_LOG(LogTemp, Warning, TEXT("Stamina : %f"), PS->GetStamina());

				if (PS->GetPlayerMovementState() == ECharacterMovementState::Exhausted) // 스태미나 확인 후 Running 상태로 전환
				{
					PlayerCharacter->SetDesiredGait(AlsGaitTags::Running);
					return;
				}
				PS->StartStaminaDrain();
				PlayerCharacter->Handle_Sprint(ActionValue);
			}

			
		}
	}
	/*
	if(Drone)
	{
		Drone Thrust Front

	}
	*/
}

void ABasePlayerController::Complete_OnSprint()
{
	UE_LOG(LogTemp, Warning, TEXT("Sprint Complete"));
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
			PlayerCharacter->SetDesiredGait(AlsGaitTags::Running);
			UE_LOG(LogTemp, Warning, TEXT("go Walking"));
			if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
			{
				PS->SetPlayerMovementState(ECharacterMovementState::Walking);
				PS->StopStaminaDrain();
				PS->StartStaminaRecovery();
			}

		}
	}
	/*
	if(Drone)
	{
		Drone Thrust Front

	}
	*/
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

	/*
	if(Drone)
		Drone Thrust back
	*/
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

void ABasePlayerController::Input_OnInteract()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}

	AActor* HitActor = TraceInteractable(1000.0f);

	if (!HitActor)
	{
		return;
	}

	// APawn 타입에 맞는 처리를 실행
	if (CachedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Interact(HitActor);
		}
	}
}

void ABasePlayerController::Input_OnStrafe(const FInputActionValue& ActionValue)
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	/*
	if(Drone)
	{
		Drone->Handle_Strafe(ActionValue);
	}
	*/
}

void ABasePlayerController::Input_OnItemUse()
{
	UE_LOG(LogTemp, Warning, TEXT("Use Item"));
	//캐릭터로 빙의가 되어서 컨트롤 되고 있는 것인지 체크
	if (!IsValid(CachedPawn))
	{
		return;
	}
	if (!(CachedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CachedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	// AItem 타입에 맞는 처리를 실행
	/*
	아이템이 무언가를 상속받는 인터페이스 클래스가 있다면
	ItemInterface->UseItem();
	*/
	//만약 총기라면
	//cameraShake();
}

void ABasePlayerController::Input_ChangeQuickSlot(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Change Quick Slot"));
	// 휠의 Y 방향만 사용 (위: +1, 아래: -1)
	const float ScrollValue = ActionValue.Get<float>();
	UE_LOG(LogTemp, Warning, TEXT("Scroll Value : %f"), ScrollValue);
	if (ScrollValue > 0.f)
	{
		// 휠 위로 → 다음 슬롯
		ChangeToNextQuickSlot();
	}
	else if (ScrollValue < 0.f)
	{
		// 휠 아래로 → 이전 슬롯
		ChangeToPreviousQuickSlot();
	}
}

void ABasePlayerController::ChangeToNextQuickSlot()
{
	CurrentQuickSlotIndex = (CurrentQuickSlotIndex + 1) % MaxQuickSlotCount;
	UpdateQuickSlotUI();
	RequestChangeItem(CurrentQuickSlotIndex);
}

void ABasePlayerController::ChangeToPreviousQuickSlot()
{
	CurrentQuickSlotIndex = (CurrentQuickSlotIndex - 1 + MaxQuickSlotCount) % MaxQuickSlotCount;
	UpdateQuickSlotUI();
	RequestChangeItem(CurrentQuickSlotIndex);
}

void ABasePlayerController::Input_SelectQuickSlot1()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	CurrentQuickSlotIndex = 0;
	UpdateQuickSlotUI();
	RequestChangeItem(CurrentQuickSlotIndex);
}

void ABasePlayerController::Input_SelectQuickSlot2()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	CurrentQuickSlotIndex = 1;
	UpdateQuickSlotUI();
	RequestChangeItem(CurrentQuickSlotIndex);
}

void ABasePlayerController::Input_SelectQuickSlot3()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	CurrentQuickSlotIndex = 2;
	UpdateQuickSlotUI();
	RequestChangeItem(CurrentQuickSlotIndex);
}

void ABasePlayerController::Input_SelectQuickSlot4()
{
	if (!IsValid(CachedPawn))
	{
		return;
	}
	CurrentQuickSlotIndex = 3;
	UpdateQuickSlotUI();
	RequestChangeItem(CurrentQuickSlotIndex);
}

void ABasePlayerController::UpdateQuickSlotUI()
{
	UE_LOG(LogTemp, Warning, TEXT("Current Slot Index : %d"), CurrentQuickSlotIndex);
	//To Do...
	//실제로 UI가 바뀌면서....
}

void ABasePlayerController::RequestChangeItem(int Itemindex)
{
	UE_LOG(LogTemp, Warning, TEXT("Request Change Item to Slot Index : %d"), Itemindex);
	//지금 슬롯에 든 걸 장착하도록 명령
	GetControlledBaseCharacter()->EquipItemFromCurrentQuickSlot(Itemindex);
}

void ABasePlayerController::Input_OpenPauseMenu()
{
	//To Do...
	//Open Pause Menu,
	//Set mouse Cursor on
	//
}

//To DO...
/*
	시점에 따른 Line Trace 시점 - 종점 컨트롤
*/
AActor* ABasePlayerController::TraceInteractable(float TraceDistance)
{
	FVector ViewLocation;
	FRotator ViewRotation;
	GetPlayerViewPoint(ViewLocation, ViewRotation);

	FVector Start = ViewLocation;
	FVector End = Start + (ViewRotation.Vector() * TraceDistance);

	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetPawn());

	bool bHit = GetWorld()->LineTraceSingleByChannel(
		Hit, Start, End, ECC_GameTraceChannel1, Params);

	// 디버그용
	DrawDebugLine(GetWorld(), Start, End, FColor::Green, false, 1.0f);

	return bHit ? Hit.GetActor() : nullptr;
}

bool ABasePlayerController::IsPossessingBaseCharacter() const
{
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn == nullptr)
	{
		return false;
	}

	return ControlledPawn->IsA(ABaseCharacter::StaticClass());
}

ABaseCharacter* ABasePlayerController::GetControlledBaseCharacter() const
{
	return Cast<ABaseCharacter>(GetPawn());
}

