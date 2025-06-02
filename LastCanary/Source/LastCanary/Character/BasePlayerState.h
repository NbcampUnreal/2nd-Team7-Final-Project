#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "BasePlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPlayerDamaged, float, CurrentHP);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerDeath);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPlayerExhausted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStaminaChanged, float, CurrentStamina);

class UAlsCharacterMovementComponent;

UCLASS()
class LASTCANARY_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABasePlayerState();

protected:
	virtual void BeginPlay() override;

public:
	// Stats
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	FPlayerStats InitialStats;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float WalkForwardSpeed{ 175.0f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float WalkBackwardSpeed{ 175.0f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float RunForwardSpeed{ 375.0f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float RunBackwardSpeed{ 375.0f };
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	float SprintSpeed{ 650.0f };

	// State
	UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentState = EPlayerState::Idle;

	UFUNCTION()
	void OnRep_CurrentState();

	UPROPERTY(BlueprintReadOnly)
	ECharacterMovementState MovementState = ECharacterMovementState::Idle;
	ECharacterMovementState GetPlayerMovementState();
	void SetPlayerMovementState(ECharacterMovementState NewState);

	// Health & Stamina
	UPROPERTY(ReplicatedUsing = OnRep_CurrentHP)
	float CurrentHP;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina)
	float CurrentStamina;

	UFUNCTION()
	void OnRep_CurrentHP();
	UFUNCTION()
	void OnRep_CurrentStamina();

	UFUNCTION()
	void ApplyDamage(float Damage);

	void ConsumeStamina(float Amount);
	void TickStaminaDrain();
	void StartStaminaDrain();
	void StopStaminaDrain();
	void StartStaminaRecovery();
	void StopStaminaRecovery();
	void StartStaminaRecoverAfterDelay();
	void StopStaminaRecoverAfterDelay();
	void TickStaminaRecovery();
	bool HasStamina() const;
	bool IsStaminaFull() const;

	FORCEINLINE float GetStamina() const { return CurrentStamina; }
	FORCEINLINE float GetHP() const { return CurrentHP; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats")
	float RecoverDelayTime = 3.0f;

private:
	FTimerHandle StaminaDrainHandle;
	FTimerHandle StaminaRecoveryHandle;
	FTimerHandle StaminaRecoveryDelayHandle;

public:
	// UI Update
	void UpdateHPUI();
	void UpdateStaminaUI();
	void UpdateDeathUI();
	void UpdateExhaustedUI();

public:
	// Events
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDamaged OnDamaged;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerDeath OnDied;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPlayerExhausted OnExhausted;
	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnStaminaChanged OnStaminaChanged;

	// Multicast
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDamaged(float HP);
	void Multicast_OnDamaged_Implementation(float HP);
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_OnDied();
	void Multicast_OnDied_Implementation();

	// Gold & Exp
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 TotalGold = 500;
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 TotalExp;
	void AddTotalGold(int32 Amount);
	void AddTotalExp(int32 Amount);
	UFUNCTION(BlueprintCallable)
	int32 GetTotalGold() const;
	UFUNCTION(BlueprintCallable)
	int32 GetTotalExp() const;

	UFUNCTION(Server, Reliable)
	void Server_SpendGold(int32 Amount);
	void Server_SpendGold_Implementation(int32 Amount);

	UFUNCTION(BlueprintCallable)
	void SetPlayerMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed);

public:
	bool bIsCharacterInHardLandingState = false;
	bool bIsCharacterInSprintingState = false;

private:
	void InitializeStats();

public:
	// Items
	UPROPERTY(Replicated)
	TArray<int32> AquiredItemIDs;

	virtual void CopyProperties(APlayerState* PlayerState) override;
};
