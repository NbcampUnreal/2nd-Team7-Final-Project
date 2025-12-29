#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterHealthComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHealthChanged);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDied);

UCLASS()
class LASTCANARY_API UCharacterHealthComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()

public:
	UCharacterHealthComponent();

protected:
	virtual void BeginPlay() override;

public:
	// 체력 회복 관련 함수
	FTimerHandle HealingTimerHandle;
	int32 HealingTicksRemaining = 0;
	float HealingPerTick = 0.f;
	void StartHealing(float TotalHealAmount, float Duration);

	void HealStep();

	void StopHealing();

public:
	// 체력 관련
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float MaxHealth = 100.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stat")
	float CurrentHealth = 100.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	bool bInfiniteHP = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	bool bIsDead = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat")
	float FallDamageThreshold = 1000.0f;
public:
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void TakeFallDamage(float Velocity);

	UFUNCTION(BlueprintCallable, Category = "Stat")
	bool IsDead() const { return CurrentHealth <= 0.f; }

	UFUNCTION(BlueprintCallable, Category = "Stat")
	void UpdateHealth();
	
	UFUNCTION(BlueprintCallable, Category = "Stat")
	void UpdateHealthUI();

	UFUNCTION(Client, Reliable, BlueprintCallable, Category = "Stat")
	void Client_UpdateHealth();
	void Client_UpdateHealth_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_PlayDamageUI();
	void Client_PlayDamageUI_Implementation();
private:
	float CalculateDamage(float ActualDamage);
	float CalculateFallDamage(float Velocity);
	void ActivateDamageCooldown(float TimeAmount);
	void ResetInvincibility();

	FTimerHandle InvincibilityTimerHandle;
	float InvincibilityTime = 0.5f;

public:
	UPROPERTY(BlueprintAssignable)
	FOnHealthChanged OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnDied OnDied;
};
