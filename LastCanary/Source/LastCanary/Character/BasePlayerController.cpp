#include "Character/BasePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BaseCharacter.h"
#include "Item/Drone/BaseDrone.h"
#include "Kismet/GameplayStatics.h"
#include "BasePlayerState.h"

#include "Net/UnrealNetwork.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

void ABasePlayerController::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("Controller Begin Play"));

	if (IsLocalController())
	{
		UE_LOG(LogTemp, Warning, TEXT("로컬 클라이언트 컨트롤러입니다."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("서버 또는 다른 클라이언트의 컨트롤러입니다."));
	}

	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		PS->OnDamaged.AddDynamic(this, &ABasePlayerController::OnCharacterDamaged);
		PS->OnDied.AddDynamic(this, &ABasePlayerController::OnCharacterDied);
		PS->OnExhausted.AddDynamic(this, &ABasePlayerController::Complete_OnSprint);
		PS->OnStaminaChanged.AddDynamic(this, &ABasePlayerController::OnStaminaUpdated);
	}

	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			UIManager->InitUIManager(this);

			if (GetPawn())
			{
				FTimerHandle HUDTimer;
				GetWorld()->GetTimerManager().SetTimer(HUDTimer,
					[this, UIManager]()
					{
						UIManager->ShowInGameHUD();
					},
					0.2f, false);
			}
		}
	}
}

void ABasePlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ABasePlayerController, SpawnedPlayerDrone);
}


void ABasePlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	InitInputComponent();
}

void ABasePlayerController::OnCharacterDamaged(float CurrentHP)
{
	// HUD 갱신하거나 효과 표시 등
	UE_LOG(LogTemp, Warning, TEXT("Player took damage!"));
	TestHP = CurrentHP;
}

void ABasePlayerController::OnCharacterDied()
{
	// HUD 갱신하거나 효과 표시 등
	UE_LOG(LogTemp, Warning, TEXT("Player die!"));
	
	//
	//TODO: 사망 전용 UI 띄우기
		//TODO: 입력 막기 등 처리

	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		CurrentPossessedPawn = SpanwedPlayerCharacter;
		//Camera위치 변경
	}
	// 현재 Pawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->SetViewMode(AlsViewModeTags::ThirdPerson);
			PlayerCharacter->HandlePlayerDeath(); //플레이어 사망처리
		}
	}
	
}

APawn* ABasePlayerController::GetMyPawn()
{
	return CurrentPossessedPawn;
}

void ABasePlayerController::SetMyPawn(APawn* NewPawn)
{
	CurrentPossessedPawn = NewPawn;
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

	if (InPawn)
	{
		SpanwedPlayerCharacter = Cast<ABaseCharacter>(InPawn);
		CurrentPossessedPawn = InPawn;
		UE_LOG(LogTemp, Warning, TEXT("OnPossess: 캐릭터 저장 완료"));
	}

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
		UE_LOG(LogTemp, Warning, TEXT("Be possessed by something other than a character"));
		//SpawnedPlayerDrone = Cast<ABaseDrone>(InPawn);
	}
	ApplyInputMappingContext(CurrentIMC);
}

void ABasePlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();

	CurrentPossessedPawn = GetPawn();
	APawn* NewPawn = GetPawn();
	if (NewPawn)
	{
		SpanwedPlayerCharacter = Cast<ABaseCharacter>(NewPawn);
		UE_LOG(LogTemp, Warning, TEXT("OnRep_Pawn: 클라이언트에서 캐릭터 저장 완료"));
	}
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

void ABasePlayerController::ClientRestart(APawn* NewPawn)
{
	Super::ClientRestart(NewPawn);

	UE_LOG(LogTemp, Warning, TEXT("ClientRestart 호출 - 클라이언트에서 빙의됨!"));
	// 여기에 빙의 완료 후 초기화 작업 가능
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
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ABasePlayerController::End_OnSprint);

		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnWalk);
		EnhancedInput->BindAction(WalkAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnWalk);

		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnCrouch);
		EnhancedInput->BindAction(CrouchAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnCrouch);

		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnJump);
		EnhancedInput->BindAction(JumpAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnJump);

		EnhancedInput->BindAction(AimAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnAim);
		EnhancedInput->BindAction(AimAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnAim);

		EnhancedInput->BindAction(ViewModeAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnViewMode);

		EnhancedInput->BindAction(InteractAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnInteract);

		EnhancedInput->BindAction(StrafeAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OnStrafe);
		EnhancedInput->BindAction(StrafeAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnStrafe);

		EnhancedInput->BindAction(ItemUseAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OnItemUse);

		EnhancedInput->BindAction(ThrowItemAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OnItemThrow);
		
		EnhancedInput->BindAction(VoiceAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OnStartedVoiceChat);
		EnhancedInput->BindAction(VoiceAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnCanceledVoiceChat);

		EnhancedInput->BindAction(ChangeShootingSettingAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_ChangeShootingSetting);
		
		EnhancedInput->BindAction(ChangeQuickSlotAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_ChangeQuickSlot);

		EnhancedInput->BindAction(SelectQuickSlot1Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot1);
		EnhancedInput->BindAction(SelectQuickSlot2Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot2);
		EnhancedInput->BindAction(SelectQuickSlot3Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot3);
		EnhancedInput->BindAction(SelectQuickSlot4Action, ETriggerEvent::Started, this, &ABasePlayerController::Input_SelectQuickSlot4);

		EnhancedInput->BindAction(OpenPauseMenuAction, ETriggerEvent::Triggered, this, &ABasePlayerController::Input_OpenPauseMenu);
		
		EnhancedInput->BindAction(ExitDroneAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_DroneExit);
	}

	ApplyInputMappingContext(InputMappingContext);
}

void ABasePlayerController::Input_OnLookMouse(const FInputActionValue& ActionValue)
{	
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_LookMouse(ActionValue);  // ABaseCharacter에 맞는 LookMouse 호출
		}
	}
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn);
		if (IsValid(Drone))
		{
			Drone->Input_Look(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnLook(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Look(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnMove(const FInputActionValue& ActionValue)
{

	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Move(ActionValue);
		}
	}
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn);
		if (IsValid(Drone))
		{
			Drone->Input_Move(ActionValue);
		}
	}
}


void ABasePlayerController::Input_OnSprint(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Sprint Triggered"));
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Sprint(ActionValue);
			if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
			{
				if (PlayerCharacter->GetDesiredGait() != AlsGaitTags::Sprinting)
				{
					bIsSprinting = false;
					PS->StopStaminaDrain();
					PS->StartStaminaRecoverAfterDelay();
				}
				else
				{
					if (PS->GetPlayerMovementState() == ECharacterMovementState::Exhausted) // 스태미나 확인 후 Running 상태로 전환
					{
						PlayerCharacter->SetDesiredGait(AlsGaitTags::Running);
						return;
					}
					PS->StopStaminaRecovery();
					PS->StopStaminaRecoverAfterDelay();
					PS->StartStaminaDrain();

					bIsSprinting = true;
				}
			}
		}
	}
}

void ABasePlayerController::End_OnSprint(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Sprint End"));
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Sprint(ActionValue);
			if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
			{
				if (bIsSprinting)
				{
					bIsSprinting = false;
					PS->StopStaminaDrain();
					PS->StartStaminaRecoverAfterDelay();
				}
				else
				{
					if (PS->GetPlayerMovementState() == ECharacterMovementState::Exhausted) // 스태미나 확인 후 Running 상태로 전환
					{
						PlayerCharacter->SetDesiredGait(AlsGaitTags::Running);
						return;
					}
					PS->StopStaminaRecovery();
					PS->StopStaminaRecoverAfterDelay();
					PS->StartStaminaDrain();

					bIsSprinting = true;
				}
			}
		}
	}
}

// 플레이어의 스태미너가 다 닳았을 때 처리되는 함수
void ABasePlayerController::Complete_OnSprint()
{
	UE_LOG(LogTemp, Warning, TEXT("Sprint Complete"));
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			//플레이어의 상태를 바꿈으로써 강제로 달리기 멈춤
			PlayerCharacter->SetDesiredGait(AlsGaitTags::Running);
			if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
			{
				//스태미너 소비 중지, 스태미너 회복
				PS->SetPlayerMovementState(ECharacterMovementState::Walking);
				PS->StopStaminaDrain();
				PS->StartStaminaRecoverAfterDelay();
			}
		}
	}
}


void ABasePlayerController::Input_OnWalk(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Walk "));
	
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	/*Hold 방식*/
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			if (WalkInputMode == EInputMode::Hold)
			{
				PlayerCharacter->Handle_Walk(ActionValue);
			}
			else if (WalkInputMode == EInputMode::Toggle)
			{
				if (bIsWalkToggled == false && ActionValue.IsNonZero())
				{
					PlayerCharacter->Handle_Walk(ActionValue);
					bIsWalkToggled = true;
				}
				else if(bIsWalkToggled == true && ActionValue.IsNonZero())
				{
					PlayerCharacter->Handle_Walk(FInputActionValue());
					bIsWalkToggled = false;
				}
			}
		}
	}
	
	
}

void ABasePlayerController::Input_OnCrouch(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Crouch"));
	const float Value = ActionValue.Get<float>();
	UE_LOG(LogTemp, Log, TEXT("Input_OnCrouch: Value = %.2f"), Value);

	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter)) 
		{
			if (CrouchInputMode == EInputMode::Hold)
			{
				PlayerCharacter->Handle_Crouch();
			}
			else if (CrouchInputMode == EInputMode::Toggle)
			{
				if (bIsWalkToggled == false && bIsCrouchKeyReleased == true)
				{
					PlayerCharacter->Handle_Crouch();
					bIsCrouchKeyReleased = false;
					bIsWalkToggled = true;
				}
				else if (bIsWalkToggled == true && bIsCrouchKeyReleased == false)
				{
					bIsCrouchKeyReleased = true;
				}
				else if (bIsWalkToggled == true && bIsCrouchKeyReleased == true)
				{
					bIsCrouchKeyReleased = false;
					PlayerCharacter->Handle_Crouch();
					bIsWalkToggled = false;
				}
				else
				{
					bIsCrouchKeyReleased = true;
				}
			}
		}
	}
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn);
		if (IsValid(Drone))
		{
			Drone->Input_MoveDown(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnJump(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Jump"));
	const float Value = ActionValue.Get<float>();

	UE_LOG(LogTemp, Log, TEXT("Input_OnJump: Value = %.2f"), Value);
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Jump(ActionValue);
		}
	}
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn);
		if (IsValid(Drone))
		{
			Drone->Input_MoveUp(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnAim(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Aim(ActionValue);
		}
	}
}

void ABasePlayerController::Input_OnViewMode()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_ViewMode();
		}
	}
}

void ABasePlayerController::Input_OnInteract()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	AActor* HitActor = TraceInteractable(1000.0f);

	if (!HitActor)
	{
		return;
	}

	
	/**/
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->Handle_Interact(HitActor);
		}
	}
}

void ABasePlayerController::Input_OnStrafe(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn);
		if (IsValid(Drone))
		{

		}
	}
}

void ABasePlayerController::Input_OnItemUse()
{
	UE_LOG(LogTemp, Warning, TEXT("Use Item"));
	//캐릭터로 빙의가 되어서 컨트롤 되고 있는 것인지 체크
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	//To Do: 아이템 추가 되고 나서...
	//if(GetHeldItem()->IsA<ABaseDrone>())
	//{
	//SpawnDrone();
	//}
	//TODO: AItem 타입에 맞는 처리를 실행
	/*
	아이템이 무언가를 상속받는 인터페이스 클래스가 있다면
	ItemInterface->UseItem();
	*/
	//test
	if (PlayerCharacter->GetCurrentQuickSlotIndex() == 0)
	{
		CameraShake();
	}
	if (PlayerCharacter->GetCurrentQuickSlotIndex() == 1)
	{
		SpawnDrone();
	}
	//만약 총기라면
	//CameraShake();
}

void ABasePlayerController::Input_OnItemThrow()
{
	UE_LOG(LogTemp, Warning, TEXT("Throw Item"));
	//TODO: Item 클래스 생기면 퀵슬롯의 아이템을 빼면서, 아이템 생성해서 힘을 주면서 밀어버리기
}

void ABasePlayerController::Input_OnStartedVoiceChat()
{
	UE_LOG(LogTemp, Warning, TEXT("Voice On"));
	//TODO: Voice 기능을 잘 추가해보기
	ConsoleCommand(TEXT("ToggleSpeaking 1"), true);
}

void ABasePlayerController::Input_OnCanceledVoiceChat()
{
	UE_LOG(LogTemp, Warning, TEXT("Voice On"));
	//TODO: Voice 기능을 잘 추가해보기
	ConsoleCommand(TEXT("ToggleSpeaking 1"), true);
}

void ABasePlayerController::Input_ChangeShootingSetting()
{
	UE_LOG(LogTemp, Warning, TEXT("Change Gun Setting"));
	//TODO: 총기 클래스 연사 단발 설정

}

void ABasePlayerController::SetShootingSetting()
{
	//TODO: 총기 클래스 들어오면 이게 눌렸을 때 총기의 Fire Setting 변화시키는 기능 추가
	// 현재 들고 있는 아이템이 총기 클래스라면
	// FireSetting 다음거로 변경
	// 총기의 속성을 변화시키는 방향이 맞아보임
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
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	
	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	SpanwedPlayerCharacter->SetCurrentQuickSlotIndex(SpanwedPlayerCharacter->GetCurrentQuickSlotIndex() + 1);
	UpdateQuickSlotUI();
}

void ABasePlayerController::ChangeToPreviousQuickSlot()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	PlayerCharacter->SetCurrentQuickSlotIndex(PlayerCharacter->GetCurrentQuickSlotIndex() - 1);
	UpdateQuickSlotUI();
}

void ABasePlayerController::Input_SelectQuickSlot1()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}	
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	PlayerCharacter->SetCurrentQuickSlotIndex(0);
	UpdateQuickSlotUI();
}

void ABasePlayerController::Input_SelectQuickSlot2()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	PlayerCharacter->SetCurrentQuickSlotIndex(1);
	UpdateQuickSlotUI();
}

void ABasePlayerController::Input_SelectQuickSlot3()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	PlayerCharacter->SetCurrentQuickSlotIndex(2);
	UpdateQuickSlotUI();
}

void ABasePlayerController::Input_SelectQuickSlot4()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	PlayerCharacter->SetCurrentQuickSlotIndex(3);
	UpdateQuickSlotUI();
}

void ABasePlayerController::UpdateQuickSlotUI()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	UE_LOG(LogTemp, Warning, TEXT("Current Slot Index : %d"), PlayerCharacter->GetCurrentQuickSlotIndex());
	//To Do: ...실제로 UI가 바뀌면서....
	//
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



//TestFunction 추후 삭제
void ABasePlayerController::OnStaminaUpdated(float NewStamina)
{
	TestStamina = NewStamina;
}

void ABasePlayerController::SetHardLandStateToPlayerState(bool flag)
{
	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{
		
		PS->bIsCharacterInHardLandingState = flag;
	}
}

void ABasePlayerController::SetSprintingStateToPlayerState(bool flag)
{
	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{

		PS->bIsCharacterInSprintingState = flag;
	}
}

void ABasePlayerController::CameraShake()
{
	AddPitchInput(1.5f);

	// 좌우(Yaw) 반동: 랜덤값만
	//TODO: 총기의 설정값에 있는 반동값을 가져오기
	
	//test용
	float YawRecoil = FMath::RandRange(-0.1f, 0.1f);
	AddYawInput(YawRecoil);

}


void ABasePlayerController::SetPlayerMovementSetting()
{
	UE_LOG(LogTemp, Warning, TEXT("SetPlayerMovementSetting"));
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (!IsValid(PlayerCharacter))
		{
			return;
		}
		ABasePlayerState* PS = GetPlayerState<ABasePlayerState>();
		if (!IsValid(PS))
		{
			return;
		}
		PlayerCharacter->SetMovementSetting(PS->WalkForwardSpeed, PS->WalkBackwardSpeed, PS->RunForwardSpeed, PS->RunBackwardSpeed, PS->SprintSpeed);
	}
}

void ABasePlayerController::ChangePlayerMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed)
{
	ABasePlayerState* PS = GetPlayerState<ABasePlayerState>();
	if (!IsValid(PS))
	{
		return;
	}
	PS->WalkForwardSpeed = _WalkForwardSpeed;
	PS->WalkBackwardSpeed = _WalkBackwardSpeed;
	PS->RunForwardSpeed = _RunForwardSpeed;
	PS->RunBackwardSpeed = _RunBackwardSpeed;
	PS->SprintSpeed = _SprintSpeed;
}

void ABasePlayerController::Input_DroneExit()
{
	UE_LOG(LogTemp, Warning, TEXT("DroneExit"));
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	// APawn 타입에 맞는 처리를 실행
	if (CurrentPossessedPawn->IsA<ABaseDrone>())
	{
		CurrentPossessedPawn = SpanwedPlayerCharacter;
		//Possess(SpanwedPlayerCharacter);
		SetViewTargetWithBlend(SpanwedPlayerCharacter, 0.5f);
		SpawnedPlayerDrone->ReturnAsItem();
		SpawnedPlayerDrone = nullptr;
		
		//TODO: 아이템 화 시켜서 바닥에 떨구기
	}
}

void ABasePlayerController::SpawnDrone()
{
	UE_LOG(LogTemp, Warning, TEXT("Spawn Drone"));
	//TODO: 플레이어 인벤토리와 아이템 구현되면...
	//	if (!IsValid(PlayerCharacter->HeldItem)) return;
	Server_SpawnDrone();
}

void ABasePlayerController::Server_SpawnDrone_Implementation()
{
	FVector Location = GetPawn()->GetActorLocation() + FVector(200, 0, 100);
	FRotator Rotation = FRotator::ZeroRotator;
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;
	Params.Owner = this;
	// ABasedrone 포인터로 받아서 타입 안전하게 캐스팅
	ABaseDrone* Drone = GetWorld()->SpawnActor<ABaseDrone>(DroneClass, Location, Rotation, Params);
	SpawnedPlayerDrone = Drone;
	SpawnedPlayerDrone->SetOwner(this);	

	//리슨서버 호스트만 
	if (HasAuthority())
	{
		PossessOnDrone();
	}
	
}

void ABasePlayerController::OnRep_SpawnedPlayerDrone()
{
	if (!IsValid(SpawnedPlayerDrone))
	{
		UE_LOG(LogTemp, Warning, TEXT("SpawnedPlayerDrone is invalid on client!"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("Client received replicated drone: %s"), *SpawnedPlayerDrone->GetName());

	// UI 연동이나 기타 클라이언트 작업만 여기서
	PossessOnDrone(); // 내부에서 서버에 possession 요청함
}

void ABasePlayerController::PossessOnDrone()
{
	if (!IsValid(SpawnedPlayerDrone))
		return;


	CurrentPossessedPawn = SpawnedPlayerDrone;
	SetViewTargetWithBlend(SpawnedPlayerDrone, 0.5f);
}