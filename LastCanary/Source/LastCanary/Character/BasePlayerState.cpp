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
    // UI 등 업데이트 가능
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
    
    // 서버에서 호출
    Multicast_OnDamaged();
    if (CurrentHP <= 0.f)
    {
        // 체력이 0이 되었을 때 처리 (죽음)
        
        UE_LOG(LogTemp, Warning, TEXT("Player has died."));
        
        // 죽었을 때 캐릭터 처리:
        Multicast_OnDied();
        
    }
}

void ABasePlayerState::Multicast_OnDamaged_Implementation()
{
    OnDamaged.Broadcast(); // 모든 클라이언트에서 브로드캐스트됨
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
        // 예: OnStaminaDepleted.Broadcast();
    }
}

void ABasePlayerState::RecoverStamina()
{
    UE_LOG(LogTemp, Warning, TEXT("Stamina : %f"), GetStamina());
    if (IsStaminaFull())
    {
        StopStaminaRecovery();
        return;
    }

    RecoverStamina(InitialStats.StaminaRecoveryRate * 0.1f);
}

void ABasePlayerState::ConsumeStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina - Amount, 0.f, InitialStats.MaxStamina);
}

void ABasePlayerState::RecoverStamina(float Amount)
{
    CurrentStamina = FMath::Clamp(CurrentStamina + Amount, 0.f, InitialStats.MaxStamina);
}

bool ABasePlayerState::HasStamina() const
{
    return CurrentStamina > 0.f;
}

bool ABasePlayerState::IsStaminaFull() const
{
    return CurrentStamina >= InitialStats.MaxStamina;
}