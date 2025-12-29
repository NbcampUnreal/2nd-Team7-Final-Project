#include "Character/Component/CharacterStaminaComponent.h"
#include "Character/BaseCharacter.h"
#include "TimerManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIElement/InGameHUD.h"

#include "LastCanary.h"

UCharacterStaminaComponent::UCharacterStaminaComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterStaminaComponent::BeginPlay()
{
	Super::BeginPlay();

	bInfiniteStamina = false;
	bIsExhausted = false;
	bCanCharacterSprint = true;
	CurrentStamina = MaxStamina;
}

void UCharacterStaminaComponent::SetMaxStamina(float _Stamina)
{
	MaxStamina = _Stamina;
}

float UCharacterStaminaComponent::GetMaxStamina()
{
	return MaxStamina;
}

void UCharacterStaminaComponent::SetStamina(float _Stamina)
{
	CurrentStamina = _Stamina;

	UpdateStaminaUI();
}

float UCharacterStaminaComponent::GetStamina()
{
	return CurrentStamina;
}

bool UCharacterStaminaComponent::CanJump()
{
	if (bIsExhausted)
	{
		return false;
	}
	
	return CurrentStamina >= JumpStaminaCost;
}

bool UCharacterStaminaComponent::CanSprint()
{
	return CurrentStamina > 0.0f;
}

void UCharacterStaminaComponent::ConsumeStamina(float Amount)
{
	if (CurrentStamina <= 0.0f)
	{
		GetBaseCharacter()->StaminaComponent->StopStaminaDrain();
		GetBaseCharacter()->StaminaComponent->StartStaminaRecoverAfterDelay();

		bIsExhausted = true;
		StopStaminaDrain();
		StartStaminaRecoverAfterDelay();
		bCanCharacterSprint = false;
		GetBaseCharacter()->bIsSprinting = false;
		GetBaseCharacter()->SetDesiredAiming(true);
		GetBaseCharacter()->SetDesiredGait(AlsGaitTags::Running);
		OnStaminaExhausted.Broadcast();
		return;
	}

	OnStaminaChanged.Broadcast();
	if (bCanCharacterSprint == false)
	{
		return;
	}
	float Stamina = FMath::Clamp(CurrentStamina - Amount, 0.f, MaxStamina);
	SetStamina(Stamina);
}

void UCharacterStaminaComponent::ConsumeStaminaOnJump()
{
	if (bInfiniteStamina)
	{
		return;
	}
	if (CurrentStamina <= 0.0f)
	{
		GetBaseCharacter()->StaminaComponent->StopStaminaDrain();
		GetBaseCharacter()->StaminaComponent->StartStaminaRecoverAfterDelay();

		bIsExhausted = true;
		StartStaminaRecoverAfterDelay();
		bCanCharacterSprint = false;
		GetBaseCharacter()->bIsSprinting = false;
		GetBaseCharacter()->SetDesiredAiming(true);
		GetBaseCharacter()->SetDesiredGait(AlsGaitTags::Running);	
		StopStaminaDrain();
		OnStaminaExhausted.Broadcast();
		return;
	}

	float Stamina = FMath::Clamp(CurrentStamina - JumpStaminaCost, 0.f, MaxStamina);
	SetStamina(Stamina);

	StopStaminaRecovery();
	StartStaminaRecoverAfterDelayOnJump();
}

void UCharacterStaminaComponent::StartStaminaDrain()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(StaminaDrainHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaminaDrainHandle,
			this,
			&UCharacterStaminaComponent::TickStaminaDrain,
			0.1f,
			true,
			0.0f);
	}
}

void UCharacterStaminaComponent::TickStaminaDrain()
{
	ConsumeStamina(ConsumedStaminaAmount);
}

void UCharacterStaminaComponent::StopStaminaDrain()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaDrainHandle);
}

void UCharacterStaminaComponent::StartStaminaRecovery()
{
	if (!GetWorld()->GetTimerManager().IsTimerActive(StaminaRecoveryHandle))
	{
		GetWorld()->GetTimerManager().SetTimer(
			StaminaRecoveryHandle,
			this,
			&UCharacterStaminaComponent::TickStaminaRecovery,
			0.1f,
			true,
			0.0f);
	}
}

void UCharacterStaminaComponent::TickStaminaRecovery()
{
	//스테미나가 가득 차 있으면 중지
	if (GetStamina() >= ExhaustionRecoveryThreshold)
	{
		bIsExhausted = false;
	}
	if (IsStaminaFull())
	{
		bIsExhausted = false;
		StopStaminaRecovery();
		return;
	}
	float Stamina = FMath::Clamp(CurrentStamina + StaminaRecoveryRate * 0.1f, 0.f, MaxStamina);
	SetStamina(Stamina);
	UpdateStaminaUI();
}

void UCharacterStaminaComponent::StopStaminaRecovery()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaRecoveryHandle);
}

void UCharacterStaminaComponent::StartStaminaRecoverAfterDelay()
{
	if (GetWorld()->GetTimerManager().IsTimerActive(StaminaRecoveryDelayHandle)) //이미 발동되었으면 넘기기
	{
		return;
	}
	GetWorld()->GetTimerManager().SetTimer(StaminaRecoveryDelayHandle, this, &UCharacterStaminaComponent::StartStaminaRecovery, RecoverDelayTime, false);
}

void UCharacterStaminaComponent::StartStaminaRecoverAfterDelayOnJump()
{
	GetWorld()->GetTimerManager().SetTimer(StaminaRecoveryDelayHandle, this, &UCharacterStaminaComponent::StartStaminaRecovery, RecoverDelayTime, false);
}

void UCharacterStaminaComponent::StopStaminaRecoverAfterDelay()
{
	GetWorld()->GetTimerManager().ClearTimer(StaminaRecoveryDelayHandle);
}

bool UCharacterStaminaComponent::HasStamina() const
{
	return CurrentStamina > 0.f;
}

bool UCharacterStaminaComponent::IsStaminaFull() const
{
	return CurrentStamina >= MaxStamina;
}

void UCharacterStaminaComponent::UpdateStaminaUI()
{
	float Percent = FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f);
	GetInGameHUD()->UpdateStaminaBar(Percent);
}