#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Character/PlayerData/PlayerDataTypes.h"
#include "BasePlayerState.generated.h"

class UAlsCharacterMovementComponent;

UCLASS()
class LASTCANARY_API ABasePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	ABasePlayerState();

protected:
	virtual void BeginPlay() override;

	UPROPERTY()
	bool bAlreadyInitialized;

public:
	////////* Default Stats *////////
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stats")
	FPlayerStats InitialStats;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxHP{ InitialStats.MaxHP };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultWalkSpeed{ InitialStats.WalkSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultRunSpeed{ InitialStats.RunSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultSprintSpeed{ InitialStats.SptintSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultCrouchSpeed{ InitialStats.CrouchSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultJumpZVelocity{ InitialStats.JumpPower };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultExhaustionRecoveryThreshold{ InitialStats.ExhaustionRecoveryThreshold };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultStaminaDrainRate{ InitialStats.StaminaDrainRate };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultStaminaRecoveryRate{ InitialStats.StaminaRecoveryRate };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultRecoverDelayTime{ InitialStats.RecoverDelayTime };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float DefaultJumpStaminaCost{ InitialStats.JumpStaminaCost };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bInfiniteStamina{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	bool bInfiniteHP{ false };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float FallDamageThreshold{ InitialStats.FallDamageThreshold };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WeightSlowdownMultiplier{ InitialStats.WeightSlowdownMultiplier };

	////////* InGame Stats *////////
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float WalkSpeed{ InitialStats.WalkSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float RunSpeed{ InitialStats.RunSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float SprintSpeed{ InitialStats.SptintSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float CrouchSpeed{ InitialStats.CrouchSpeed };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float JumpZVelocity{ InitialStats.JumpPower };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float ExhaustionRecoveryThreshold{ InitialStats.ExhaustionRecoveryThreshold };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float StaminaDrainRate{ InitialStats.StaminaDrainRate }; // 초당 15 소모

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float StaminaRecoveryRate{ InitialStats.StaminaRecoveryRate }; // 초당 30 회복

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float RecoverDelayTime{ InitialStats.RecoverDelayTime };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float JumpStaminaCost{ InitialStats.JumpStaminaCost };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxSpirit{ InitialStats.MaxSpirit };

	//InputMode
	//인풋모드 변경(Toggle, Hold)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputSettings")
	EInputMode SprintInputMode = EInputMode::Hold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputSettings")
	EInputMode WalkInputMode = EInputMode::Hold;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "InputSettings")
	EInputMode CrouchInputMode = EInputMode::Hold;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundSettings")
	float RunningFootSoundModifier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundSettings")
	float WalkingFootSoundModifier = 0.3f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundSettings")
	float SprintingFootSoundModifier = 1.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "SoundSettings")
	float CrouchingFootSoundModifier = 0.2f;

public:
	UPROPERTY(Replicated, BlueprintReadWrite)
	FString PlayerInGameName = "Default";

	void SetPlayerInGameName(FString Name);
	FString GetInGameName();


public:
	// State
	UPROPERTY(ReplicatedUsing = OnRep_bHasEscaped)
	bool bHasEscaped = false;

	UFUNCTION()
	void OnRep_bHasEscaped();

	void MarkAsEscaped();

	UPROPERTY(ReplicatedUsing = OnRep_CurrentState, VisibleAnywhere, BlueprintReadOnly, Category = "State")
	EPlayerState CurrentState = EPlayerState::Idle;

	EPlayerInGameStatus InGameState = EPlayerInGameStatus::Alive;

	void SetInGameStatus(EPlayerInGameStatus Status);
	EPlayerInGameStatus GetInGameStatus();


	UFUNCTION()
	void OnRep_CurrentState();

	UPROPERTY(BlueprintReadOnly)
	ECharacterMovementState MovementState = ECharacterMovementState::Idle;
	ECharacterMovementState GetPlayerMovementState();
	void SetPlayerMovementState(ECharacterMovementState NewState);
	void SetPlayerMovementSetting(float _WalkForwardSpeed, float _WalkBackwardSpeed, float _RunForwardSpeed, float _RunBackwardSpeed, float _SprintSpeed);
	// Health & Stamina
	UPROPERTY(Replicated)
	float CurrentHP;
	UPROPERTY(ReplicatedUsing = OnRep_CurrentStamina)
	float CurrentStamina;

	UPROPERTY(Replicated)
	float CurrentSpirit;

	UFUNCTION()
	void OnRep_CurrentStamina();

	UFUNCTION()
	void ApplyDamage(float Damage);

	FORCEINLINE float GetStamina() const { return CurrentStamina; }
	FORCEINLINE float GetHP() const { return CurrentHP; }
	FORCEINLINE float GetSpirit() const { return CurrentSpirit; }
	
	void SetStamina(float NewStamina);
	void SetHP(float NewHP);
	void SetSpirit(float NewSpirit);
	
	UFUNCTION(Client, Reliable)
	void Client_UpdateSpirit(float NewSpirit);
	void Client_UpdateSpirit_Implementation(float NewSpirit);

	UFUNCTION(Client, Reliable)
	void Client_UpdateHP(float NewHP);
	void Client_UpdateHP_Implementation(float NewHP);

public:
	// UI Update
	void UpdateHPUI();
	void UpdateStaminaUI();
	void UpdateDeathUI();
	void UpdateExhaustedUI();
	void UpdateSpiritUI();

public:
	// Gold & Exp
	UPROPERTY(Replicated, BlueprintReadOnly)
	int32 TotalGold = 500000;
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

	// 자원 수집 내역 (RowName → 개수)
	TMap<FName, int32> CollectedResourceMap;

	void AddCollectedResource(FName RowName);

	const TMap<FName, int32>& GetCollectedResourceMap() const;

	void ClearCollectedResources();
};
