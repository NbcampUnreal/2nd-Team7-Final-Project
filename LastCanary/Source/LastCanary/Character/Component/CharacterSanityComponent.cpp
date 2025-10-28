#include "Character/Component/CharacterSanityComponent.h"
#include "Character/BaseCharacter.h"
#include "Engine/DamageEvents.h"
#include "Character/BasePlayerState.h"
#include "Kismet/GameplayStatics.h"
#include "Character/Component/CharacterStaminaComponent.h"

#include "LastCanary.h"

UCharacterSanityComponent::UCharacterSanityComponent()
{
	CurrentSanity = MaxSanity;
}

void UCharacterSanityComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitializePanicActions();
	
	// 패시브 자연 감소 시작
	StartPassiveSanityDecay();
}

void UCharacterSanityComponent::InitializePanicActions()
{
	if (PanicActions.Num() > 0) return; // 이미 초기화 됐으면 스킵

	PanicActions.Add([this]() { PlayScreamSound_Local(); });
	PanicActions.Add([this]() { TriggerPanicVoice(PanicDuration); });
	PanicActions.Add([this]() { UseItemUnexpectedly(); });
	PanicActions.Add([this]() { PlaySighSoundForAll(); });
	PanicActions.Add([this]() { ForceSetMouseSensitivity(PanicSensitivity, PanicDuration); });
	PanicActions.Add([this]() { ForceInvertMouseTemporary(true, PanicDuration); });
	PanicActions.Add([this]() { PlayHallucinationSound(); });
}

int32 UCharacterSanityComponent::GetMaxPanicActionForStage(int32 Stage) const
{
	if (PanicActions.Num() == 0)
		return 0;

	// Stage 0~4를 0~MaxIndex 범위로 매핑
	int32 ClampedStage = FMath::Clamp(Stage, 0, 4);
	int32 MaxIndex = FMath::RoundToInt((PanicActions.Num() / 4.f) * ClampedStage);

	// 최소 1개 이상은 선택 가능하도록
	return FMath::Clamp(MaxIndex, 0, PanicActions.Num());
}

void UCharacterSanityComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

float UCharacterSanityComponent::TakeSanityDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!IsValid(GetBaseCharacter()) || !GetBaseCharacter()->HasAuthority())
	{
		return 0.0f;
	}
		
	float FinalDamage = CalculateTakeSanityDamage(DamageAmount);
	CurrentSanity = FMath::Clamp(CurrentSanity - FinalDamage, 0.0f, MaxSanity);

	// ----- 단계 계산 -----
	int32 NewStage = 0;

	if (CurrentSanity >= 80.f)        NewStage = 0; // 정상
	else if (CurrentSanity >= 60.f)   NewStage = 1;
	else if (CurrentSanity >= 40.f)   NewStage = 2;
	else if (CurrentSanity >= 20.f)   NewStage = 3;
	else                              NewStage = 4; // 0~19

	// ----- 단계 변경 감지 -----
	if (NewStage != CurrentSanityStage)
	{
		CurrentSanityStage = NewStage;
		ApplySanityStageEffects(CurrentSanityStage);
	}

	// ----- 0 이하 특수 효과 -----
	if (CurrentSanity <= 0.0f && !bZeroSanityEffectsApplied)
	{
		bZeroSanityEffectsApplied = true;
		StartHPDecayAtZeroSanity();
		if (bReduceStaminaAtZero) 
		{
			ApplyHalfStamina();
		}
	}

	return DamageAmount;
}

float UCharacterSanityComponent::RestoreSanity(float Amount)
{
	if (!GetBaseCharacter()->HasAuthority())
	{
		return 0;
	}

	CurrentSanity = FMath::Clamp(CurrentSanity + Amount, 0.0f, MaxSanity);

	if (CurrentSanity > 0.f && bZeroSanityEffectsApplied)
	{
		StopHPDecay();
		RestoreHalfStamina();
		bZeroSanityEffectsApplied = false;
	}

	if (CurrentSanity > PanicTriggerThreshold)
	{
		ExitPanicState();
	}

	return Amount;
}

void UCharacterSanityComponent::ApplySanityStageEffects(int32 Stage)
{
	StopPanicBehaviorLoop(); // 이전 단계 효과 정리

	CurrentSanityStage = Stage;

	// 반복 간격은 단계별로 조절
	switch (Stage)
	{
	case 0: RepeatRate = 0.f; break;
	case 1: RepeatRate = 10.f; break;
	case 2: RepeatRate = 7.f; break;
	case 3: RepeatRate = 5.f; break;
	case 4: RepeatRate = 3.f; break;
	}

	// 단계별 PanicBehavior 시작
	if (RepeatRate > 0.f)
	{
		StartPanicBehaviorLoop();
	}
}

void UCharacterSanityComponent::TriggerSanityTickDamage()
{
	GetWorld()->GetTimerManager().SetTimer(
		SanityTickDamageHandle,
		this,
		&UCharacterSanityComponent::TakeSanityTickDamage,
		SanityDamageTickInterval,
		true,
		0.01f
	);
}

void UCharacterSanityComponent::TakeSanityTickDamage()
{
	FDamageEvent Dmg;
	TakeSanityDamage(SanityTickDamage, Dmg, nullptr, nullptr);
}

float UCharacterSanityComponent::CalculateTakeSanityDamage(float DamageAmount)
{
	return DamageAmount;
}

//------------------ Sanity Passive Decay ------------------------

void UCharacterSanityComponent::StartPassiveSanityDecay()
{
	GetWorld()->GetTimerManager().SetTimer(
		PassiveSanityDecayHandle,
		this,
		&UCharacterSanityComponent::PassiveSanityDecayTick,
		PassiveSanityDecayInterval,
		true
	);
}

void UCharacterSanityComponent::StopPassiveSanityDecay()
{
	GetWorld()->GetTimerManager().ClearTimer(PassiveSanityDecayHandle);
}

void UCharacterSanityComponent::PassiveSanityDecayTick()
{
	FDamageEvent Dmg;
	TakeSanityDamage(PassiveSanityDecayAmount, Dmg, nullptr, nullptr);
}

//------------------ HP Decay at Zero Sanity ----------------------

void UCharacterSanityComponent::StartHPDecayAtZeroSanity()
{
	GetWorld()->GetTimerManager().SetTimer(
		HPDecayHandle,
		this,
		&UCharacterSanityComponent::HPDecayTick,
		HPDecayInterval,
		true
	);
}

void UCharacterSanityComponent::StopHPDecay()
{
	GetWorld()->GetTimerManager().ClearTimer(HPDecayHandle);
}

void UCharacterSanityComponent::HPDecayTick()
{
	FDamageEvent Dmg;
	GetBaseCharacter()->TakeDamage(HPDecayAmountPerTick, Dmg, nullptr, GetBaseCharacter());
}

//------------------ Stamina Half at Zero -------------------------------

void UCharacterSanityComponent::ApplyHalfStamina()
{
	if (IsValid(GetBaseCharacter()))
	{
		OriginalStaminaMax = GetBaseCharacter()->StaminaComponent->GetMaxStamina();
		GetBaseCharacter()->StaminaComponent->SetMaxStamina(OriginalStaminaMax * 0.5f);
	}
}

void UCharacterSanityComponent::RestoreHalfStamina()
{
	if (IsValid(GetBaseCharacter()) && OriginalStaminaMax > 0.f)
	{
		GetBaseCharacter()->StaminaComponent->SetMaxStamina(OriginalStaminaMax);
	}
}

//------------------ Panic State Existing -------------------------------

void UCharacterSanityComponent::EnterPanicState()
{
	Client_EnterPanicState();
}

void UCharacterSanityComponent::ExitPanicState()
{
	Client_ExitPanicState();
}

void UCharacterSanityComponent::Client_EnterPanicState_Implementation()
{
	StartPanicBehaviorLoop();
}

void UCharacterSanityComponent::Client_ExitPanicState_Implementation()
{
	StopPanicBehaviorLoop();
}

void UCharacterSanityComponent::StartPanicBehaviorLoop()
{
	GetWorld()->GetTimerManager().SetTimer(
		PanicActionTimerHandle,
		this,
		&UCharacterSanityComponent::PerformRandomPanicAction,
		RepeatRate,
		true,
		InitialDelay
	);
}

void UCharacterSanityComponent::StopPanicBehaviorLoop()
{
	GetWorld()->GetTimerManager().ClearTimer(PanicActionTimerHandle);
}

void UCharacterSanityComponent::PerformRandomPanicAction()
{
	int32 MaxActions = GetMaxPanicActionForStage(CurrentSanityStage);
	if (MaxActions <= 0) return;

	int32 RandomIndex = FMath::RandRange(0, MaxActions - 1);
	if (PanicActions.IsValidIndex(RandomIndex))
	{
		PanicActions[RandomIndex]();
	}
}


//------------------- Existing Local/Network Methods --------------------

void UCharacterSanityComponent::PlayScreamSound_Local()
{
	if (GetBaseCharacter()->IsLocallyControlled() && ScreamSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ScreamSound, GetBaseCharacter()->GetActorLocation());
	}
}

void UCharacterSanityComponent::UseItemUnexpectedly()
{
	GetBaseCharacter()->UseEquippedItem(1.0f);
	GetBaseCharacter()->UseEquippedItem(0.0f);
}

void UCharacterSanityComponent::PlaySighSoundForAll()
{
	if (GetBaseCharacter()->IsLocallyControlled())
	{
		Server_PlaySighSound();
	}
}

void UCharacterSanityComponent::Server_PlaySighSound_Implementation()
{
	if (GetBaseCharacter()->HasAuthority())
	{
		Multicast_PlaySighSound();
	}
}

void UCharacterSanityComponent::Multicast_PlaySighSound_Implementation()
{
	if (SighSound)
	{
		UGameplayStatics::SpawnSoundAttached(
			SighSound,
			GetBaseCharacter()->GetRootComponent(),
			NAME_None,
			FVector::ZeroVector,
			EAttachLocation::KeepRelativeOffset,
			true
		);
	}
}

//------------------- Hallucination / Vision ----------------------------

void UCharacterSanityComponent::PlayHallucinationSound()
{
	if (GetBaseCharacter()->IsLocallyControlled() && HallucinationSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, HallucinationSound, GetBaseCharacter()->GetActorLocation());
	}
}

void UCharacterSanityComponent::ApplyVisionNoiseEffect()
{
	// PostProcess 비주얼 노이즈 적용 자리 (추후 구현)
}

void UCharacterSanityComponent::RemoveVisionNoiseEffect()
{
	// PostProcess 노이즈 제거 (추후 구현)
}

//------------------- Mouse / Input Disturbance -------------------------

void UCharacterSanityComponent::ForceSetMouseSensitivity(float NewSensitivity, float Duration)
{
	GetBaseCharacter()->MouseSensitivityMultiplier = NewSensitivity;

	GetWorld()->GetTimerManager().ClearTimer(MouseSensitivityRestoreHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MouseSensitivityRestoreHandle,
		this,
		&UCharacterSanityComponent::RestoreOriginalMouseSensitivity,
		Duration,
		false
	);
}

void UCharacterSanityComponent::RestoreOriginalMouseSensitivity()
{
	GetBaseCharacter()->MouseSensitivityMultiplier = 1.0f;
}

void UCharacterSanityComponent::ForceInvertMouse(bool bInvert)
{
	GetBaseCharacter()->MouseInvertMultiplier = bInvert ? -1.0f : 1.0f;
}

void UCharacterSanityComponent::ForceInvertMouseTemporary(bool bInvert, float Duration)
{
	ForceInvertMouse(bInvert);

	GetWorld()->GetTimerManager().ClearTimer(MouseInvertResetTimerHandle);
	GetWorld()->GetTimerManager().SetTimer(
		MouseInvertResetTimerHandle,
		this,
		&UCharacterSanityComponent::RestoreMouseInvert,
		Duration,
		false
	);
}

void UCharacterSanityComponent::RestoreMouseInvert()
{
	ForceInvertMouse(false);
}

//------------------- Panic Voice --------------------------------------

void UCharacterSanityComponent::TriggerPanicVoice(float Duration)
{
	EnterPanicVoice();
	GetWorld()->GetTimerManager().ClearTimer(PanicVoiceDurationHandle);
	GetWorld()->GetTimerManager().SetTimer(
		PanicVoiceDurationHandle,
		this,
		&UCharacterSanityComponent::ExitPanicVoice,
		Duration,
		false
	);
}
