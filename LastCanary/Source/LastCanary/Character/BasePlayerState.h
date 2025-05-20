// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "BasePlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDamaged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerExhausted);


UCLASS()
class LASTCANARY_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()
	
protected:
    ABasePlayerState();

    virtual void BeginPlay() override;

public:	
    /** 초기 스탯 (디자인용) */
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
    FPlayerStats InitialStats;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentState = EPlayerState::Idle;


    UPROPERTY(BlueprintReadOnly)
    ECharacterMovementState MovementState = ECharacterMovementState::Idle;

    ECharacterMovementState GetPlayerMovementState();
    void SetPlayerMovementState(ECharacterMovementState NewState);


    /** 현재 HP (실시간) */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
    float CurrentHP;

    /** 현재 스태미나 (실시간) */
    UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina)
    float CurrentStamina;

public:
    void StartStaminaDrain();
    void StopStaminaDrain();

    void StartStaminaRecovery();
    void StopStaminaRecovery();

    bool HasStamina() const;
    void ConsumeStamina(float Amount);
    void RecoverStamina(float Amount);
    bool IsStaminaFull() const;

    float GetStamina() const { return CurrentStamina; }

private:
    FTimerHandle StaminaDrainHandle;
    FTimerHandle StaminaRecoveryHandle;

    void DrainStamina();
    void RecoverStamina();


public:
    UFUNCTION()
    void OnRep_CurrentHP();

    UFUNCTION()
    void OnRep_CurrentStamina();

    UFUNCTION()
    void ApplyDamage(float Damage);
    

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerDamaged OnDamaged;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerDeath OnDied;

    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnPlayerExhausted OnExhausted;

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDamaged();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_OnDied();

public:
    /** 서버 전용: 스탯 초기화 */
    void InitializeStats();




    /* 게임 계정 전용 스탯 */
public:
    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 TotalGold;

    UPROPERTY(Replicated, BlueprintReadOnly)
    int32 TotalExp;


};
