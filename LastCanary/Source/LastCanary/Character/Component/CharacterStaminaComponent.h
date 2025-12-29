#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterStaminaComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaExhausted);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaRecover);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStaminaThresholdReached);

class ABaseCharacter;

UCLASS()
class LASTCANARY_API UCharacterStaminaComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
	UCharacterStaminaComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 스태미나 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxStamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentStamina = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float ConsumedStaminaAmount = 1.f;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void SetMaxStamina(float _Stamina);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetMaxStamina();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void SetStamina(float _Stamina);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	float GetStamina();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool CanJump();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool CanSprint();
public:
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ConsumeStamina(float Amount);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void ConsumeStaminaOnJump();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StartStaminaDrain();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void TickStaminaDrain();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StopStaminaDrain();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StartStaminaRecovery();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void TickStaminaRecovery();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StopStaminaRecovery();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StartStaminaRecoverAfterDelay();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StartStaminaRecoverAfterDelayOnJump();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void StopStaminaRecoverAfterDelay();

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool HasStamina() const;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool IsStaminaFull() const;

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void UpdateStaminaUI();
public:
	UPROPERTY(BlueprintAssignable)
	FOnStaminaChanged OnStaminaChanged;

	UPROPERTY(BlueprintAssignable)
	FOnStaminaExhausted OnStaminaExhausted;

	UPROPERTY(BlueprintAssignable)
	FOnStaminaRecover OnStaminaRecover;

	UPROPERTY(BlueprintAssignable)
	FOnStaminaThresholdReached OnStaminaThresholdReached;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	bool bInfiniteStamina = false;

public:
	FTimerHandle StaminaDrainHandle;
	FTimerHandle StaminaRecoveryHandle;
	FTimerHandle StaminaRecoveryDelayHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float StaminaDrainRate = 10.f; // 초당 10 소모

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float StaminaRecoveryRate = 50.f; // 초당 50 회복

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float RecoverDelayTime = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float JumpStaminaCost = 10.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stamina")
	float ExhaustionRecoveryThreshold = 30.0f;

	bool bIsExhausted = false;

	bool bCanCharacterSprint = true;
};
