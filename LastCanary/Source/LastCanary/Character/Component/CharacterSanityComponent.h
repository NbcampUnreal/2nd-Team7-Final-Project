#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "CharacterSanityComponent.generated.h"


UCLASS()
class LASTCANARY_API UCharacterSanityComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()
public:
	UCharacterSanityComponent();

protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Panic 액션 후보 풀
	TArray<TFunction<void()>> PanicActions;

	void InitializePanicActions();

	// 단계별 최대 랜덤 액션 개수
	int32 GetMaxPanicActionForStage(int32 Stage) const;

	//-------------------------------------------------------
	// Sanity 수치
	//-------------------------------------------------------
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Stats")
	float CurrentSanity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Stats")
	float MaxSanity = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Stats")
	float PanicTriggerThreshold = 10.f;
	
	//*정신력 단계 관련*//
	UPROPERTY(BlueprintReadOnly)
	int32 CurrentSanityStage = 0;

	//-------------------------------------------------------
	// 패시브 Sanity 감소
	//-------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|PassiveDecay")
	float PassiveSanityDecayAmount = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|PassiveDecay")
	float PassiveSanityDecayInterval = 2.f;

	FTimerHandle PassiveSanityDecayHandle;

	//-------------------------------------------------------
	// Sanity 0일 때 HP 감소 + 스태미나 절반
	//-------------------------------------------------------
	UPROPERTY(EditAnywhere, Category = "Sanity|ZeroEffects")
	float HPDecayAmountPerTick = 1.f;

	UPROPERTY(EditAnywhere, Category = "Sanity|ZeroEffects")
	float HPDecayInterval = 1.f;

	FTimerHandle HPDecayHandle;
	bool bZeroSanityEffectsApplied = false;

	UPROPERTY(EditAnywhere, Category = "Sanity|ZeroEffects")
	bool bReduceStaminaAtZero = true;

	float OriginalStaminaMax = 0.f;

	//-------------------------------------------------------
	// Panic / 사운드 / 환청
	//-------------------------------------------------------
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Sounds")
	USoundBase* ScreamSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Sounds")
	USoundBase* SighSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|Sounds")
	USoundBase* HallucinationSound;

	//-------------------------------------------------------
	// Panic 반복 타이머 관련
	//-------------------------------------------------------
	FTimerHandle SanityTickDamageHandle;
	FTimerHandle PanicActionTimerHandle;
	float RepeatRate = 5.0f;
	float InitialDelay = 10.0f;
	float SanityTickDamage = 1.0f;
	float SanityDamageTickInterval = 1.0f;
	FTimerHandle PanicVoiceDurationHandle;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|PanicState")
	float PanicSensitivity = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Sanity|PanicState")
	float PanicDuration = 5.0f;

	FTimerHandle MouseSensitivityRestoreHandle;
	FTimerHandle MouseInvertResetTimerHandle;

	//-------------------------------------------------------
	// 메인 API
	//-------------------------------------------------------
public:
	float TakeSanityDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

	float RestoreSanity(float Amount);

	void ApplySanityStageEffects(int32 Stage);

	UFUNCTION(BlueprintImplementableEvent)
	void EnterPanicVoice();

	UFUNCTION(BlueprintImplementableEvent)
	void ExitPanicVoice();

	UFUNCTION(BlueprintCallable)
	void TriggerSanityTickDamage();

	void TakeSanityTickDamage();

	float CalculateTakeSanityDamage(float DamageAmount);

	//-------------------------------------------------------
	// Passive & Zero State
	//-------------------------------------------------------
	void StartPassiveSanityDecay();
	void StopPassiveSanityDecay();
	void PassiveSanityDecayTick();

	void StartHPDecayAtZeroSanity();
	void StopHPDecay();
	void HPDecayTick();

	void ApplyHalfStamina();
	void RestoreHalfStamina();

	//-------------------------------------------------------
	// PanicState
	//-------------------------------------------------------
	
	void EnterPanicState();
	void ExitPanicState();

	UFUNCTION(Client, Reliable)
	void Client_EnterPanicState();
	void Client_EnterPanicState_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_ExitPanicState();
	void Client_ExitPanicState_Implementation();

	void StartPanicBehaviorLoop();
	void StopPanicBehaviorLoop();
	void PerformRandomPanicAction();

	void PlayScreamSound_Local();
	void UseItemUnexpectedly();
	void PlaySighSoundForAll();

	UFUNCTION(Server, Reliable)
	void Server_PlaySighSound();
	void Server_PlaySighSound_Implementation();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlaySighSound();
	void Multicast_PlaySighSound_Implementation();

	//-------------------------------------------------------
	// Hallucination & 시야 노이즈 효과용
	//-------------------------------------------------------
	void PlayHallucinationSound();
	void ApplyVisionNoiseEffect();
	void RemoveVisionNoiseEffect();

	//-------------------------------------------------------
	// 마우스 조작
	//-------------------------------------------------------
	void ForceSetMouseSensitivity(float NewSensitivity, float Duration);
	void RestoreOriginalMouseSensitivity();
	void ForceInvertMouse(bool bInvert);
	void ForceInvertMouseTemporary(bool bInvert, float Duration);
	void RestoreMouseInvert();

	void TriggerPanicVoice(float Duration);
};
