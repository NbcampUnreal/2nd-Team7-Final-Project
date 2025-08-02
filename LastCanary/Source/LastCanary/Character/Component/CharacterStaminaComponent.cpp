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

	if (GetCharacter())
	{
		//GetCharacter()->OnJumpStarted.AddDynamic(this, &UCharacterStaminaComponent::HandleJumpStart);
	}

	bInfiniteStamina = false;
	bIsExhausted = false;
	bCanCharacterSprint = true;
	CurrentStamina = MaxStamina;
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
		bIsExhausted = true;
		StopStaminaRecoverAfterDelay();
		OnStaminaExhausted.Broadcast();
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
		bIsExhausted = true;
		StopStaminaRecoverAfterDelay();
		OnStaminaExhausted.Broadcast();
	}

	OnStaminaChanged.Broadcast();

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
	// 컴포넌트가 붙은 캐릭터 얻기
	if (CachedCharacter)
	{
		// 그 캐릭터를 소유한 컨트롤러 얻기
		if (APlayerController* PC = Cast<APlayerController>(CachedCharacter->GetController()))
		{
			if (ULCGameInstanceSubsystem* Subsystem = GetWorld()->GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
			{
				if (ULCUIManager* UIManager = Subsystem->GetUIManager())
				{
					if (UInGameHUD* HUD = UIManager->GetInGameHUD())
					{
						float Percent = FMath::Clamp(CurrentStamina / MaxStamina, 0.0f, 1.0f);
						HUD->UpdateStaminaBar(Percent);
					}
				}
			}
		}
	}
}