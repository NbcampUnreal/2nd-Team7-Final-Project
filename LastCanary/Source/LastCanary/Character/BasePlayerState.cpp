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
}

void ABasePlayerState::BeginPlay()
{
	Super::BeginPlay();
	
	if (bAlreadyInitialized)
	{
		return;
	}
	bAlreadyInitialized = true;

	if (CurrentHP <= 0.f && InitialStats.MaxHP > 0.f)  // 새 생성일 경우만
	{
		InitializeStats();
	}
	UpdateHPUI();
	UpdateStaminaUI();

	if (ABaseCharacter* MyCharacter = Cast<ABaseCharacter>(GetPawn()))
	{
		MyCharacter->SetMovementSetting();
	}
}

void ABasePlayerState::SetInGameStatus(EPlayerInGameStatus Status)
{
	InGameState = Status;
}
EPlayerInGameStatus ABasePlayerState::GetInGameStatus()
{
	return InGameState;
}

void ABasePlayerState::InitializeStats()
{
	CurrentHP = InitialStats.MaxHP;
	CurrentStamina = InitialStats.MaxStamina;

}
void ABasePlayerState::SetStamina(float NewStamina)
{
	CurrentStamina = NewStamina; 
	UpdateStaminaUI(); 
}
void ABasePlayerState::SetHP(float NewHP) 
{
	if (HasAuthority())
	{	//서버는 바로 바꾸기
		CurrentHP = NewHP; 
		Client_UpdateHP(NewHP);
	}
}

void ABasePlayerState::Client_UpdateHP_Implementation(float NewHP)
{
	CurrentHP = NewHP;
	UpdateHPUI();
}

void ABasePlayerState::OnRep_CurrentStamina()
{
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
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()))
	{
		if (ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			if (ULCUIManager* UIManager = Subsystem->GetUIManager())
			{

			}
		}
	}
}

void ABasePlayerState::UpdateExhaustedUI()
{
	// TODO: implement HUD->OnPlayerExhausted(); if needed
}

void ABasePlayerState::ApplyDamage(float Damage)
{
	////여기는 서버에서 처리
	CurrentHP = FMath::Clamp(CurrentHP - Damage, 0.f, MaxHP);
	Client_UpdateHP(CurrentHP);
}

void ABasePlayerState::OnRep_bHasEscaped()
{
	LOG_Frame_WARNING(TEXT("[OnRep_bHasEscaped] Replicated value: %s"), bHasEscaped ? TEXT("TRUE") : TEXT("FALSE"));

	// 여기서 UI 업데이트 등도 가능
	if (bHasEscaped)
	{
		LOG_Frame_WARNING(TEXT("탈출 성공 상태 클라이언트 반영됨!"));
	}
}

void ABasePlayerState::MarkAsEscaped()
{
	bHasEscaped = true;
}

void ABasePlayerState::OnRep_CurrentState()
{

}
ECharacterMovementState ABasePlayerState::GetPlayerMovementState()
{
	return MovementState;
}

void ABasePlayerState::SetPlayerMovementState(ECharacterMovementState NewState)
{
	MovementState = NewState;
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
			_SprintSpeed,
			_RunForwardSpeed / 2);
	}
}

void ABasePlayerState::CopyProperties(APlayerState* PlayerState)
{
	Super::CopyProperties(PlayerState);
	LOG_Frame_WARNING(TEXT("CopyProperties called for ABasePlayerState"));

	if (ABasePlayerState* TargetState = Cast<ABasePlayerState>(PlayerState))
	{
		// 유지할 데이터
		TargetState->AquiredItemIDs = AquiredItemIDs;
		TargetState->TotalGold = TotalGold;
		TargetState->TotalExp = TotalExp;

		// 초기화할 데이터
		TargetState->CurrentHP = TargetState->MaxHP; 
		TargetState->CurrentStamina = TargetState->InitialStats.MaxStamina;
	}
}

void ABasePlayerState::AddCollectedResource(FName RowName)
{
	if (CollectedResourceMap.Contains(RowName))
	{
		CollectedResourceMap[RowName]++;
	}
	else
	{
		CollectedResourceMap.Add(RowName, 1);
	}
}

const TMap<FName, int32>& ABasePlayerState::GetCollectedResourceMap() const
{
	return CollectedResourceMap;
}

void ABasePlayerState::ClearCollectedResources()
{
	CollectedResourceMap.Empty();
}

void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABasePlayerState, CurrentHP);
	DOREPLIFETIME(ABasePlayerState, CurrentStamina);
	DOREPLIFETIME(ABasePlayerState, TotalGold);
	DOREPLIFETIME(ABasePlayerState, TotalExp);
	DOREPLIFETIME(ABasePlayerState, CurrentState);
	DOREPLIFETIME(ABasePlayerState, AquiredItemIDs);
}
