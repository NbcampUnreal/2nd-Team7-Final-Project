#include "Character/Component/CharacterHealthComponent.h"
#include "Character/BaseCharacter.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/InGameHUD.h"

#include "LastCanary.h"

UCharacterHealthComponent::UCharacterHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterHealthComponent::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	bIsDead = false;
}

void UCharacterHealthComponent::StartHealing(float TotalHealAmount, float Duration)
{
	if (!GetBaseCharacter()->HasAuthority())
	{
		return;
	}

	if (GetWorld()->GetTimerManager().IsTimerActive(HealingTimerHandle))
	{
		return;
	}

	const float Interval = 1.0f;
	HealingTicksRemaining = FMath::CeilToInt(Duration / Interval);
	HealingPerTick = TotalHealAmount / HealingTicksRemaining;

	GetWorld()->GetTimerManager().SetTimer(HealingTimerHandle, this, &UCharacterHealthComponent::HealStep, Interval, true);
}

void UCharacterHealthComponent::HealStep()
{
	if (!GetBaseCharacter()->HasAuthority())
	{
		return;
	}

	const float NewHP = FMath::Clamp(CurrentHealth + HealingPerTick, 0.0f, MaxHealth);
	CurrentHealth = NewHP;

	HealingTicksRemaining--;

	if (HealingTicksRemaining <= 0)
	{
		StopHealing();
	}
}

void UCharacterHealthComponent::StopHealing()
{
	GetWorld()->GetTimerManager().ClearTimer(HealingTimerHandle);
	HealingTicksRemaining = 0;
}

void UCharacterHealthComponent::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float CalculatedHP = CalculateDamage(DamageAmount);
	if (bInfiniteHP == true)
	{
		return;
	}

	CurrentHealth = FMath::Clamp(CurrentHealth - CalculatedHP, 0.f, MaxHealth);
	
	if (CalculatedHP > 0.f)
	{
		ActivateDamageCooldown(InvincibilityTime); // 0.5초간 무적
		UpdateHealth();
		Client_PlayDamageUI();
	}

	if (CurrentHealth <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnDied.Broadcast(); // 사망 이벤트 발생
	}
	OnHealthChanged.Broadcast();
}

void UCharacterHealthComponent::TakeFallDamage(float Velocity)
{
	float CalculatedHP = CalculateFallDamage(Velocity);

	CurrentHealth = FMath::Clamp(CurrentHealth - CalculatedHP, 0.f, MaxHealth);
	if (CalculatedHP > 0.f)
	{
		ActivateDamageCooldown(InvincibilityTime); // 0.5초간 무적
		UpdateHealth();
		Client_PlayDamageUI();
	}

	if (CurrentHealth <= 0.f && !bIsDead)
	{
		bIsDead = true;
		OnDied.Broadcast(); // 사망 이벤트 발생
	}
	OnHealthChanged.Broadcast();
}

float UCharacterHealthComponent::CalculateDamage(float ActualDamage)
{
	return ActualDamage;
}

float UCharacterHealthComponent::CalculateFallDamage(float Velocity)
{
	if (-Velocity < FallDamageThreshold)
	{
		return 0.0f;
	}
	float AppliedFallDamage = -Velocity - FallDamageThreshold;
	float AdditionalCalculatedDamage = AppliedFallDamage / 10;
	//TODO: 여기에다가 추가로 뭔가 장비나 방어력이 추가 되면 여기서 계산하고 넘겨도 됨.
	return AdditionalCalculatedDamage;
}

void UCharacterHealthComponent::ActivateDamageCooldown(float TimeAmount)
{
	bInfiniteHP = true;
	GetWorld()->GetTimerManager().SetTimer(InvincibilityTimerHandle, this, &UCharacterHealthComponent::ResetInvincibility, TimeAmount, false);
}

void UCharacterHealthComponent::ResetInvincibility()
{
	bInfiniteHP = false;
}

void UCharacterHealthComponent::UpdateHealth()
{
	Client_UpdateHealth(); // 서버가 아닌 클라이언트의 캐릭터에서 처리할 것
	/*
Client_PlayHitSound();
MyPlayerState->Client_PlayDamageUI();
*/
}

void UCharacterHealthComponent::Client_UpdateHealth_Implementation()
{
	UpdateHealthUI();
}

void UCharacterHealthComponent::UpdateHealthUI()
{
	float Percent = FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f);
	GetInGameHUD()->UpdateHPBar(Percent);
}

void UCharacterHealthComponent::Client_PlayDamageUI_Implementation()
{
	GetInGameHUD()->PlayTakeDamageAnim();
}
