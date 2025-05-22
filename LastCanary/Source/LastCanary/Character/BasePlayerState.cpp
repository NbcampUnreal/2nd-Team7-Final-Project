// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/BasePlayerState.h"
#include "BaseCharacter.h"
#include "Net/UnrealNetwork.h"

ABasePlayerState::ABasePlayerState()
{
    bReplicates = true;
}

void ABasePlayerState::BeginPlay()
{
    InitializeStats();
    UE_LOG(LogTemp, Warning, TEXT("PlayerState is Begin"));
}

void ABasePlayerState::InitializeStats()
{
    CurrentHP = InitialStats.MaxHP;
    CurrentStamina = InitialStats.MaxStamina;
}

void ABasePlayerState::OnRep_CurrentHP()
{
    // UI 등 업데이트 가능
}

void ABasePlayerState::OnRep_CurrentStamina()
{
    // 컨트롤러에게 알림
    OnStaminaChanged.Broadcast(CurrentStamina);
}

void ABasePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);


    DOREPLIFETIME(ABasePlayerState, CurrentHP);
    DOREPLIFETIME(ABasePlayerState, CurrentStamina);

    DOREPLIFETIME(ABasePlayerState, TotalGold);
    DOREPLIFETIME(ABasePlayerState, TotalExp);
}

void ABasePlayerState::ApplyDamage(float Damage)
{
    CurrentHP = FMath::Clamp(CurrentHP  - Damage, 0.f, InitialStats.MaxHP);
    UE_LOG(LogTemp, Warning, TEXT("Player State HP update."));
    //OnDamaged.Broadcast(CurrentHP);
    // 서버에서 호출
    Multicast_OnDamaged();
    if (CurrentHP <= 0.f)
    {
        // 체력이 0이 되었을 때 처리 (죽음)
        CurrentState = EPlayerState::Dead;
        UE_LOG(LogTemp, Warning, TEXT("Player has died."));
        
        // 죽었을 때 캐릭터 처리:
        Multicast_OnDied();
        //OnDied.Broadcast();
    }
}

void ABasePlayerState::Multicast_OnDamaged_Implementation()
{
    OnDamaged.Broadcast(CurrentHP); // 모든 클라이언트에서 브로드캐스트됨
}

void ABasePlayerState::Multicast_OnDied_Implementation()
{
    OnDied.Broadcast(); // 모든 클라이언트에서 브로드캐스트됨
}

ECharacterMovementState ABasePlayerState::GetPlayerMovementState()
{
    return MovementState;
}

void ABasePlayerState::SetPlayerMovementState(ECharacterMovementState NewState)
{
    MovementState = NewState;
}


void ABasePlayerState::StartStaminaDrain()
{
    if (!GetWorldTimerManager().IsTimerActive(StaminaDrainHandle))
    {
        GetWorldTimerManager().SetTimer(StaminaDrainHandle, this, &ABasePlayerState::DrainStamina, 0.1f, true);
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
        GetWorldTimerManager().SetTimer(StaminaRecoveryHandle, this, &ABasePlayerState::RecoverStamina, 0.1f, true);
    }
}

void ABasePlayerState::StopStaminaRecovery()
{
    GetWorldTimerManager().ClearTimer(StaminaRecoveryHandle);
    SetPlayerMovementState(ECharacterMovementState::Walking);
}

void ABasePlayerState::DrainStamina()
{
    if (CurrentStamina <= 0.f)
    {
        StopStaminaDrain();
        return;
    }

    ConsumeStamina(InitialStats.StaminaDrainRate * 0.1f);

    if (CurrentStamina <= 0.f)
    {
        SetPlayerMovementState(ECharacterMovementState::Exhausted);
        StartStaminaRecovery();
        // Notify exhausted
        UE_LOG(LogTemp, Warning, TEXT("Notify Exhausted"));
        OnExhausted.Broadcast();
    }
}

void ABasePlayerState::RecoverStamina()
{
    if (IsStaminaFull())
    {
        StopStaminaRecovery();
        return;
    }

    RecoverStamina(InitialStats.StaminaRecoveryRate * 0.1f);
}

void ABasePlayerState::StartStaminaRecoverAfterDelay()
{
    GetWorldTimerManager().SetTimer(StaminaRecoveryDelayHandle, this, &ABasePlayerState::StartStaminaRecovery, RecoverDelayTime, false);
}

void ABasePlayerState::StopStaminaRecoverAfterDelay()
{
    GetWorldTimerManager().ClearTimer(StaminaRecoveryDelayHandle);
}

void ABasePlayerState::ConsumeStamina(float Amount)
{
    if (bIsCharacterInHardLandingState || !bIsCharacterInSprintingState)
    {
        return;
    }
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, InitialStats.MaxStamina);
    OnStaminaChanged.Broadcast(CurrentStamina);
}

void ABasePlayerState::RecoverStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.f, InitialStats.MaxStamina);
    OnStaminaChanged.Broadcast(CurrentStamina);
}

bool ABasePlayerState::HasStamina() const
{
    return CurrentStamina > 0.f;
}

bool ABasePlayerState::IsStaminaFull() const
{
    return CurrentStamina >= InitialStats.MaxStamina;
}


int32 ABasePlayerState::GetTotalGold()
{
    return TotalGold;
}

int32 ABasePlayerState::GetTotalExp()
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