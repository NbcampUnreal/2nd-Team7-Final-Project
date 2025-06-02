#include "Character/BasePlayerState.h"
#include "BaseCharacter.h"
#include "Net/UnrealNetwork.h"
#include "ALS/Public/AlsCharacterMovementComponent.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/InGameHUD.h"

#include "LastCanary.h"

ABasePlayerState::ABasePlayerState()
{
	bReplicates = true;
	LOG_Frame_WARNING(TEXT("ABasePlayerState::ABasePlayerState()ABasePlayerState::ABasePlayerState()"));

}

void ABasePlayerState::BeginPlay()
{
	LOG_Frame_WARNING(TEXT("ABasePlayerState::BeginPlay()"));
	InitializeStats();
	UpdateHPUI();
	UpdateStaminaUI();
}

void ABasePlayerState::InitializeStats()
{
	CurrentHP = InitialStats.MaxHP;
	CurrentStamina = InitialStats.MaxStamina;
}

void ABasePlayerState::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(CurrentHP);
	LOG_Char_WARNING(TEXT("OnRep_CurrentHP"));
	UpdateHPUI();
}

void ABasePlayerState::OnRep_CurrentStamina()
{
	OnStaminaChanged.Broadcast(CurrentStamina);
	LOG_Char_WARNING(TEXT("OnRep_CurrentStamina"));
	UpdateStaminaUI();
}

void ABasePlayerState::UpdateHPUI()
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = Subsystem->GetUIManager())
			{
				if (UInGameHUD* HUD = UIManager->GetInGameHUD())
				{
					HUD->UpdateLowHealthEffect(CurrentHP, InitialStats.MaxHP);
				}
			}
		}
	}
}

void ABasePlayerState::UpdateStaminaUI()
{
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = Subsystem->GetUIManager())
			{
				if (UInGameHUD* HUD = UIManager->GetInGameHUD())
				{
					float Percent = FMath::Clamp(CurrentStamina / InitialStats.MaxStamina, 0.0f, 1.0f);
					HUD->UpdateStaminaBar(Percent);
				}
			}
		}
	}
}

void ABasePlayerState::UpdateDeathUI()
{
	// TODO: implement HUD->OnPlayerDeath(); if needed
}

void ABasePlayerState::UpdateExhaustedUI()
{
	// TODO: implement HUD->OnPlayerExhausted(); if needed
}

void ABasePlayerState::ApplyDamage(float Damage)
{
	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.f, InitialStats.MaxHP);

	if (IsOwnedBy(GetWorld()->GetFirstPlayerController()))
	{
		UpdateHPUI();
	}

	// Multicast_OnDamaged();

	if (CurrentHP <= 0.f)
	{
		CurrentState = EPlayerState::Dead;

		if (IsOwnedBy(GetWorld()->GetFirstPlayerController()))
		{
			UpdateDeathUI();
		}

		Multicast_OnDied();
	}
}

void ABasePlayerState::Multicast_OnDamaged_Implementation()
{
	OnDamaged.Broadcast(CurrentHP);
	if (!IsOwnedBy(GetWorld()->GetFirstPlayerController()))
	{
		UpdateHPUI();
	}
}

void ABasePlayerState::Multicast_OnDied_Implementation()
{
	OnDied.Broadcast();
	if (!IsOwnedBy(GetWorld()->GetFirstPlayerController()))
	{
		UpdateDeathUI();
	}
}

ECharacterMovementState ABasePlayerState::GetPlayerMovementState()
{
	return MovementState;
}

void ABasePlayerState::SetPlayerMovementState(ECharacterMovementState NewState)
{
	MovementState = NewState;
}

void ABasePlayerState::TickStaminaDrain()
{
	ConsumeStamina(InitialStats.StaminaDrainRate * 0.1f);
}

void ABasePlayerState::StartStaminaDrain()
{
	if (!GetWorldTimerManager().IsTimerActive(StaminaDrainHandle))
	{
		GetWorldTimerManager().SetTimer(
			StaminaDrainHandle,
			this,
			&ABasePlayerState::TickStaminaDrain,
			0.1f,
			true);
	}
	StopStaminaRecovery();
}

void ABasePlayerState::StopStaminaDrain()
{
	GetWorldTimerManager().ClearTimer(StaminaDrainHandle);
}

void ABasePlayerState::StartStaminaRecovery()
{
	if (!GetWorldTimerManager().IsTimerActive(StaminaRecoveryHandle))
	{
		GetWorldTimerManager().SetTimer(
			StaminaRecoveryHandle,
			this,
			&ABasePlayerState::TickStaminaRecovery,
			0.1f,
			true);
	}
}

void ABasePlayerState::StopStaminaRecovery()
{
	GetWorldTimerManager().ClearTimer(StaminaRecoveryHandle);
	SetPlayerMovementState(ECharacterMovementState::Walking);
}

void ABasePlayerState::StartStaminaRecoverAfterDelay()
{
	GetWorldTimerManager().SetTimer(StaminaRecoveryDelayHandle, this, &ABasePlayerState::StartStaminaRecovery, RecoverDelayTime, false);
}

void ABasePlayerState::StopStaminaRecoverAfterDelay()
{
	GetWorldTimerManager().ClearTimer(StaminaRecoveryDelayHandle);
}

void ABasePlayerState::TickStaminaRecovery()
{
	if (IsStaminaFull())
	{
		StopStaminaRecovery();
		return;
	}
	CurrentStamina = FMath::Clamp(CurrentStamina + InitialStats.StaminaRecoveryRate * 0.1f, 0.f, InitialStats.MaxStamina);
	
	if (IsOwnedBy(GetWorld()->GetFirstPlayerController()))
	{
		UpdateStaminaUI();
	}
	OnStaminaChanged.Broadcast(CurrentStamina);
}

void ABasePlayerState::ConsumeStamina(float Amount)
{
	if (bIsCharacterInHardLandingState || !bIsCharacterInSprintingState)
	{
		return;
	}

	CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, InitialStats.MaxStamina);
	UE_LOG(LogTemp, Warning, TEXT("stamina : %f"), CurrentStamina);
	if (IsOwnedBy(GetWorld()->GetFirstPlayerController()))
	{
		UpdateStaminaUI();
	}
	OnStaminaChanged.Broadcast(CurrentStamina);

	if (CurrentStamina <= 0.f)
	{
		SetPlayerMovementState(ECharacterMovementState::Exhausted);
		StartStaminaRecovery();

		if (IsOwnedBy(GetWorld()->GetFirstPlayerController()))
		{
			UpdateExhaustedUI();
		}
		OnExhausted.Broadcast();
	}
}

bool ABasePlayerState::HasStamina() const
{
	return CurrentStamina > 0.f;
}

bool ABasePlayerState::IsStaminaFull() const
{
	return CurrentStamina >= InitialStats.MaxStamina;
}

int32 ABasePlayerState::GetTotalGold() const
{
	return TotalGold;
}

int32 ABasePlayerState::GetTotalExp() const
{
	return TotalExp;
}

void ABasePlayerState::AddTotalGold(int32 Amount)
{
	TotalGold += Amount;
}

void ABasePlayerState::AddTotalExp(int32 Amount)
{
	TotalExp += Amount;
}

void ABasePlayerState::Server_SpendGold_Implementation(int32 Amount)
{
	if (Amount <= 0)
	{
		return;
	}

	if (TotalGold >= Amount)
	{
		TotalGold -= Amount;
		LOG_Frame_WARNING(TEXT("Gold spent: %d, Remaining: %d"), Amount, TotalGold);
	}
	else
	{
		LOG_Frame_WARNING(TEXT("Not enough gold. Required: %d, Have: %d"), Amount, TotalGold);
	}
}

void ABasePlayerState::SetPlayerMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed)
{
	if (ABaseCharacter* MyCharacter = Cast<ABaseCharacter>(GetPawn()))
	{
		MyCharacter->AlsCharacterMovement->SetGaitSettings(
			_WalkForwardSpeed,
			_WalkBackwardSpeed,
			_RunForwardSpeed,
			_RunBackwardSpeed,
			_SprintSpeed);
	}
}

void ABasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	LOG_Frame_WARNING(TEXT("CopyProperties called for ABasePlayerState"));
	if (ABasePlayerState* TargetState = Cast<ABasePlayerState>(PlayerState))
	{
		TargetState->AquiredItemIDs = AquiredItemIDs;
		TargetState->TotalGold = TotalGold;
		TargetState->TotalExp = TotalExp;
		// 필요한 데이터 더 복사 가능
		TargetState->CurrentHP = CurrentHP; // 테스트용으로 추가
		LOG_Frame_WARNING(TEXT("TotalGold: %d, TotalExp: %d"), TotalGold, TotalExp);
	}
}

void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, CurrentHP);
	DOREPLIFETIME(ABasePlayerState, CurrentStamina);
	DOREPLIFETIME(ABasePlayerState, TotalGold);
	DOREPLIFETIME(ABasePlayerState, TotalExp);
	DOREPLIFETIME(ABasePlayerState, AquiredItemIDs);
}
