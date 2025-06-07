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

void ABasePlayerController::OnExitGate()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in OnExitGate"));
		return;
	}

	if (HasAuthority())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnExitGate : is on Server"));
		HandleExitGate(); // 서버 전용 로직 실행
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("OnExitGate : is on Client"));
		Server_OnExitGate(); // 클라에서는 서버 RPC만 호출
	}
}

void ABasePlayerController::Server_OnExitGate_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("Server_OnExitGate_Implementation"));
	HandleExitGate(); // 서버에서 실행
}

void ABasePlayerController::HandleExitGate()
{
	UE_LOG(LogTemp, Warning, TEXT("HandleExitGate"));
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	if (!IsValid(PlayerCharacter))
	{
		return;
	}
	PlayerCharacter->EscapeThroughGate();
}

void ABasePlayerController::OnPlayerExitActivePlay()
{
	// 일정 시간 후 Pawn 제거
	Client_OnPlayerExitActivePlay();
	//클라이언트에서 해야할 것.
	APawn* MyPawn = GetPawn();
	if (MyPawn)
	{
		MyPawn->DetachFromControllerPendingDestroy();
		MyPawn->SetLifeSpan(5.f); // 또는 Custom Fade Out
	}
	OnUnPossess();
}

void ABasePlayerController::Client_OnPlayerExitActivePlay_Implementation()
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
			PlayerCharacter->SetCameraMode(false);
		}
	}
	//UnPossess();
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
	}
	// Pawn의 타입에 따라 MappingContext를 자동 변경
	if (InPawn->IsA(ABaseCharacter::StaticClass()))
	{
		CurrentIMC = InputMappingContext;
		Cast<ABaseCharacter>(InPawn)->SetPossess(true);
		SpanwedPlayerCharacter = Cast<ABaseCharacter>(InPawn);
	}
	else
	{
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
	}
}

void ABasePlayerController::OnUnPossess()
{
	APawn* PossessedPawn = GetPawn();  // Possess 해제 전에 캐싱

	Super::OnUnPossess();

	CachedPawn = nullptr;
	CurrentPossessedPawn = nullptr;

	// SpanwedPlayerCharacter 초기화 여부는 프로젝트 설계에 따라 결정
	// SpanwedPlayerCharacter = nullptr;
	SpawnedPlayerDrone = nullptr;

	//RemoveInputMappingContext(CurrentIMC);

	if (PossessedPawn && PossessedPawn->IsA(ABaseCharacter::StaticClass()))
	{
		Cast<ABaseCharacter>(PossessedPawn)->SetPossess(false);
	}
}


void ABasePlayerController::ClientRestart(APawn* NewPawn)
{
	Super::ClientRestart(NewPawn);
}

void ABasePlayerController::InitInputComponent()
{
	EnhancedInput = Cast<UEnhancedInputComponent>(InputComponent);

	if (!IsValid(EnhancedInput))
	{
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
	ABasePlayerState* MyPlayerState = GetPlayerState<ABasePlayerState>();
	if (!IsValid(MyPlayerState))
	{
		return;
	}

	if (MyPlayerState->InGameState == EPlayerInGameStatus::Spectating)
	{
		//UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is Spectating"));
		const auto Value{ ActionValue.Get<FVector2D>() };
		if (Value.X != 0.0f)
		{
			if (bIsSpectatingButtonClicked == true)
			{
				return;
			}
			bIsSpectatingButtonClicked = true;
			if (Value.X > 0.0f)
			{
				SpectateNextPlayer();
			}
			else
			{
				SpectatePreviousPlayer();
			}
		}
		else
		{
			bIsSpectatingButtonClicked = false;
		}
		return;
	}

	if (!IsValid(CurrentPossessedPawn))
	{
		//UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnMove"));
		return;
	}
	

	// APawn 타입에 맞는 처리를 실행
	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Move(ActionValue);
	}
	else if (ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn))
	{
		Drone->Input_Move(ActionValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Unknown pawn type in Input_OnMove: %s"), *CurrentPossessedPawn->GetName());
	}
}


void ABasePlayerController::Input_OnSprint(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnSprint"));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Sprint(ActionValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is not an ABaseCharacter: %s"), *CurrentPossessedPawn->GetName());
	}
}

void ABasePlayerController::Input_OnWalk(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnWalk"));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Walk(ActionValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is not an ABaseCharacter: %s"), *CurrentPossessedPawn->GetName());
	}
}

void ABasePlayerController::Input_OnCrouch(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnCrouch"));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Crouch(ActionValue);
	}
	else if (ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn))
	{
		Drone->Input_MoveDown(ActionValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn has no valid input handler for crouch: %s"), *CurrentPossessedPawn->GetName());
	}
}

void ABasePlayerController::Input_OnJump(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnJump"));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Jump(ActionValue);
	}
	else if (ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn))
	{
		Drone->Input_MoveUp(ActionValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn has no valid input handler for jump: %s"), *CurrentPossessedPawn->GetName());
	}
}

void ABasePlayerController::Input_OnAim(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Aim(ActionValue);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is not an ABaseCharacter: %s"), *CurrentPossessedPawn->GetName());
	}
}


void ABasePlayerController::Input_OnViewMode()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnViewMode"));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_ViewMode();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is not an ABaseCharacter: %s"), *CurrentPossessedPawn->GetName());
	}
}


void ABasePlayerController::Input_OnInteract()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_OnInteract"));
		return;
	}

	AActor* HitActor = TraceInteractable(1000.0f);
	if (!HitActor)
	{
		UE_LOG(LogTemp, Warning, TEXT("No interactable actor found in trace."));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Interact();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is not an ABaseCharacter: %s"), *CurrentPossessedPawn->GetName());
	}
}


void ABasePlayerController::Input_Reload()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is invalid in Input_Reload"));
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->Handle_Reload();
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("CurrentPossessedPawn is not an ABaseCharacter: %s"), *CurrentPossessedPawn->GetName());
	}
}

void ABasePlayerController::Input_OnStrafe(const FInputActionValue& ActionValue)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	// TODO: 추후 테스트용 코드 제거 필요
	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		UE_LOG(LogTemp, Warning, TEXT("Toggle Inventory"));
		PlayerCharacter->ToggleInventory();
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
			if (MyPS && MyPS->InGameState != EPlayerInGameStatus::Spectating)  // 실제 게임 진행 중인 플레이어 관전
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
			if (MyPS && MyPS->InGameState == EPlayerInGameStatus::Spectating)
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

	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->UseEquippedItem();
	}
}


void ABasePlayerController::Input_OnItemThrow()
{
	UE_LOG(LogTemp, Warning, TEXT("Throw Item"));
	
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->DropCurrentItem();
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
	// 휠의 Y 방향만 사용 (위: +1, 아래: -1)
	const float ScrollValue = ActionValue.Get<float>();
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

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->SetCurrentQuickSlotIndex(PlayerCharacter->GetCurrentQuickSlotIndex() + 1);
		UpdateQuickSlotUI();
	}
}

void ABasePlayerController::ChangeToPreviousQuickSlot()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->SetCurrentQuickSlotIndex(PlayerCharacter->GetCurrentQuickSlotIndex() - 1);
		UpdateQuickSlotUI();
	}
}


void ABasePlayerController::SelectQuickSlot(int32 SlotIndex)
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		if (IsValid(PlayerCharacter->CurrentInteractMontage))
		{
			return;
		}
		PlayerCharacter->SetCurrentQuickSlotIndex(SlotIndex);
		UpdateQuickSlotUI();
	}
}

void ABasePlayerController::Input_SelectQuickSlot1()
{
	SelectQuickSlot(0);
}

void ABasePlayerController::Input_SelectQuickSlot2()
{
	SelectQuickSlot(1);
}

void ABasePlayerController::Input_SelectQuickSlot3()
{
	SelectQuickSlot(2);
}

void ABasePlayerController::Input_SelectQuickSlot4()
{
	SelectQuickSlot(3);
}

void ABasePlayerController::UpdateQuickSlotUI()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		// To Do: ...실제로 UI가 바뀌면서....
		//
	}
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
	ABasePlayerState* PS = GetPlayerState<ABasePlayerState>();
	if (!IsValid(PS))
	{
		return;
	}
	PS->bIsCharacterInHardLandingState = flag;
}

void ABasePlayerController::SetSprintingStateToPlayerState(bool flag)
{
	if (ABasePlayerState* PS = GetPlayerState<ABasePlayerState>())
	{

		PS->bIsCharacterInSprintingState = flag;
	}
}


void ABasePlayerController::Input_DroneExit()
{
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}

	if (ABaseDrone* Drone = Cast<ABaseDrone>(CurrentPossessedPawn))
	{
		CurrentPossessedPawn = SpanwedPlayerCharacter;
		//Possess(SpanwedPlayerCharacter);
		SetViewTargetWithBlend(SpanwedPlayerCharacter, 0.5f);

		if (SpawnedPlayerDrone)
		{
			SpawnedPlayerDrone->ReturnAsItem();
			SpawnedPlayerDrone = nullptr;
		}
	}
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->SwapHeadMaterialTransparent(true);
	}
}


void ABasePlayerController::SpawnDrone()
{
	//TODO: 플레이어 인벤토리와 아이템 구현되면...
	//	if (!IsValid(PlayerCharacter->HeldItem)) return;
	Server_SpawnDrone();
}

void ABasePlayerController::Server_SpawnDrone_Implementation()
{
	FVector Location = GetPawn()->GetActorLocation() + FVector(0, 0, 200);
	FRotator Rotation = GetPawn()->GetActorRotation();
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

	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn))
	{
		PlayerCharacter->SwapHeadMaterialTransparent(false);
	}

	CurrentPossessedPawn = SpawnedPlayerDrone;

	SpawnedPlayerDrone->SetCharacterLocation(SpanwedPlayerCharacter->GetActorLocation());
	

	
	SetViewTargetWithBlend(SpawnedPlayerDrone, 0.5f);
}

void ABasePlayerController::CameraSetOnScope()
{

}

void ABasePlayerController::InteractGimmick(ALCBaseGimmick* Target)
{
	if (!IsValid(Target))
	{
		return;		
	}
	if (!IsValid(CurrentPossessedPawn))
	{
		return;
	}
	if (!(CurrentPossessedPawn->IsA<ABaseCharacter>()))
	{
		return;
	}
	ABaseCharacter* PlayerCharacter = Cast<ABaseCharacter>(CurrentPossessedPawn);
	Server_InteractWithGimmick(Target);
	PlayerCharacter->PlayInteractionMontage(Target);
}

void ABasePlayerController::Server_InteractWithGimmick_Implementation(ALCBaseGimmick* Target)
{
	if (!IsValid(Target))
	{
		return;
	}


	Target->SetOwner(this);

	IInteractableInterface::Execute_Interact(Target, this);
}