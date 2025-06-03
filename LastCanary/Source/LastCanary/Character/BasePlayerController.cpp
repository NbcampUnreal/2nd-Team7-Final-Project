#include "Character/BasePlayerController.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "BaseCharacter.h"
#include "Item/Drone/BaseDrone.h"
#include "Kismet/GameplayStatics.h"
#include "BasePlayerState.h"
#include "Framework/GameState/LCGameState.h"
#include "Net/UnrealNetwork.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Actor/Gimmick/LCBaseGimmick.h"



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
	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			// UIManager->InitUIManager(this);

			if (GetPawn())
			{
				FTimerHandle HUDTimer;
				GetWorld()->GetTimerManager().SetTimer(HUDTimer,
					[this, UIManager]()
					{
						UIManager->ShowInGameHUD();
					},
					0.5f, false);
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

	//InitInputComponent();
}

void ABasePlayerController::OnCharacterDied()
{
	//클라이언트에서 해야할 것.
	Client_OnCharacterDied();
}

void ABasePlayerController::Client_OnCharacterDied_Implementation()
{
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
		}
	}
	//CreateWidget();
	//addtoviewport
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
		EnhancedInput->BindAction(SprintAction, ETriggerEvent::Canceled, this, &ABasePlayerController::Input_OnSprint);

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

		EnhancedInput->BindAction(ItemUseAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OnItemUse);

		EnhancedInput->BindAction(ThrowItemAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_OnItemThrow);

		EnhancedInput->BindAction(RifleReloadAction, ETriggerEvent::Started, this, &ABasePlayerController::Input_Reload);

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
		}
	}
}

void ABasePlayerController::Input_OnWalk(const FInputActionValue& ActionValue)
{
	UE_LOG(LogTemp, Warning, TEXT("Walk"));
	
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
			PlayerCharacter->Handle_Interact();
		}
	}
}

void ABasePlayerController::Input_Reload()
{

	UE_LOG(LogTemp, Warning, TEXT("input Reload"));
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{

			UE_LOG(LogTemp, Warning, TEXT("Reload"));
			PlayerCharacter->Handle_Reload();
		}
	}
}

void ABasePlayerController::Input_OnStrafe(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (MyPlayerState)
	{
		if (MyPlayerState->CurrentState == EPlayerState::Dead)
		{
			const float Input = ActionValue.Get<float>();
			if (Input > 0.5f)
			{
				SpectateNextPlayer();
			}
			else
			{
				SpectatePreviousPlayer();
			}
		}
		return;
	}

	//나중에 사용으로 빼긴 할 건데 일단 테스트용으로 넣어놔서 아쉽게도 호출이 되는 그런 코드.
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		UE_LOG(LogTemp, Warning, TEXT("Toggle Inventory"));
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{			
			PlayerCharacter->ToggleInventory();
		}
	}
}


void ABasePlayerController::SpectatePreviousPlayer()
{
	TArray<ABasePlayerState*> PlayerList = GetPlayerArray();
	int32 PlayerListLength = PlayerList.Num();
	if (PlayerListLength <= 0)
	{
		return;
	}

	CurrentSpectatedCharacterIndex--;
	if (CurrentSpectatedCharacterIndex < 0)
	{
		CurrentSpectatedCharacterIndex = PlayerListLength - 1;
	}
	SetViewTargetWithBlend(PlayerList[CurrentSpectatedCharacterIndex]->GetPawn());
}
void ABasePlayerController::SpectateNextPlayer()
{
	TArray<ABasePlayerState*> PlayerList = GetPlayerArray();
	int32 PlayerListLength = PlayerList.Num();
	if (PlayerListLength <= 0)
	{
		return;
	}

	CurrentSpectatedCharacterIndex++;
	CurrentSpectatedCharacterIndex %= PlayerListLength;
	SetViewTargetWithBlend(PlayerList[CurrentSpectatedCharacterIndex]->GetPawn());
}
TArray<ABasePlayerState*> ABasePlayerController::GetPlayerArray()
{
	ALCGameState* GameState = GetWorld()->GetGameState<ALCGameState>();
	if (!IsValid(GameState))
	{
		return SpectatorTargets;
	}

	//아마 처음 죽었을 때 이 분기를 타게 될 것임
	if (SpectatorTargets.IsEmpty())
	{
		for (APlayerState* PS : GameState->PlayerArray)
		{
			ABasePlayerState* MyPS = Cast<ABasePlayerState>(PS);
			if (MyPS && MyPS->CurrentState != EPlayerState::Dead)  // 살아있는 플레이어 필터
			{
				SpectatorTargets.Add(MyPS);
			}
		}
		return SpectatorTargets;
	}
	else
	{
		TArray<ABasePlayerState*> ToRemove;
		for (ABasePlayerState* MyPS : SpectatorTargets)
		{
			if (MyPS && MyPS->CurrentState == EPlayerState::Dead)
			{
				ToRemove.Add(MyPS);
			}
		}
		for (ABasePlayerState* MyPS : ToRemove)
		{
			SpectatorTargets.Remove(MyPS);
		}
		return SpectatorTargets;
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
	
	PlayerCharacter->UseEquippedItem();
}

void ABasePlayerController::Input_OnItemThrow()
{
	UE_LOG(LogTemp, Warning, TEXT("Throw Item"));
	if (CurrentPossessedPawn->IsA<ABaseCharacter>())
	{
		ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
		if (IsValid(PlayerCharacter))
		{
			PlayerCharacter->DropCurrentItem();;
		}
	}
}

void ABasePlayerController::Input_OnStartedVoiceChat()
{
	UE_LOG(LogTemp, Warning, TEXT("Voice On"));
	//TODO: Voice 기능을 잘 추가해보기
	ConsoleCommand(TEXT("ToggleSpeaking 1"), true);
}

void ABasePlayerController::Input_OnCanceledVoiceChat()
{
	UE_LOG(LogTemp, Warning, TEXT("Voice Off"));
	//TODO: Voice 기능을 잘 추가해보기
	ConsoleCommand(TEXT("ToggleSpeaking 0"), true);
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
	if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		if (ULCUIManager* UIManager = Subsystem->GetUIManager())
		{
			if (UIManager->IsPauseMenuOpen())
			{
				UIManager->HidePauseMenu();				
			}
			else
			{
				UIManager->ShowPauseMenu();
			}
		}
	}
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
	// 새로운 반동량을 기존 값에 누적
	RecoilStepPitch += 1.5f / RecoilMaxSteps;
	RecoilStepYaw += FMath::RandRange(-YawRecoilRange, YawRecoilRange) / RecoilMaxSteps;

	// 타이머가 안 돌고 있을 때만 시작
	if (!GetWorld()->GetTimerManager().IsTimerActive(RecoilTimerHandle))
	{
		RecoilStep = 0;
		GetWorld()->GetTimerManager().SetTimer(RecoilTimerHandle, this, &ABasePlayerController::ApplyRecoilStep, 0.02f, true);
	}
}

void ABasePlayerController::ApplyRecoilStep()
{
	AddPitchInput(RecoilStepPitch);
	AddYawInput(RecoilStepYaw);

	RecoilStep++;

	if (RecoilStep >= RecoilMaxSteps)
	{
		// 마지막 단계에서 값을 0으로 초기화
		RecoilStepPitch = 0.0f;
		RecoilStepYaw = 0.0f;
		GetWorld()->GetTimerManager().ClearTimer(RecoilTimerHandle);
	}
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
	FVector Location = GetPawn()->GetActorLocation() + FVector(0, 0, 200);
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

	SpawnedPlayerDrone->SetCharacterLocation(SpanwedPlayerCharacter->GetActorLocation());
	
	SetViewTargetWithBlend(SpawnedPlayerDrone, 0.5f);
}

void ABasePlayerController::CameraSetOnScope()
{

}

void ABasePlayerController::InteractGimmick(ALCBaseGimmick* Target)
{
	if (IsValid(Target))
	{
		Server_InteractWithGimmick(Target);
	}
}

void ABasePlayerController::Server_InteractWithGimmick_Implementation(ALCBaseGimmick* Target)
{
	Target->Server_ActivateGimmick();
}