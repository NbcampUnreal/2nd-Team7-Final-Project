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

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float CurrentSpirit;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxSpirit = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float PanicTriggerThreshold = 10.f;

    FTimerHandle SpiritTickDamageHandle;
    FTimerHandle PanicActionTimerHandle;
    float RepeatRate = 5.0f;
    float InitialDelay = 10.0f;
    float SpiritTickDamage = 1.0f;
    float SpiritDamageTickInterval = 1.0f;
    FTimerHandle PanicVoiceDurationHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PanicState")
    USoundBase* ScreamSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PanicState")
    USoundBase* SighSound;

    FTimerHandle MouseSensitivityRestoreHandle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PanicState")
    float PanicSensitivity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "PanicState")
    float PanicDuration = 5.0f;

    FTimerHandle MouseInvertResetTimerHandle;

public:
    float TakeSpiritDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser);

    UFUNCTION(BlueprintImplementableEvent)
    void EnterPanicVoice();

    UFUNCTION(BlueprintImplementableEvent)
    void ExitPanicVoice();

    UFUNCTION(BlueprintCallable)
    void TriggerSpiritTickDamage();

    void TakeSpiritTickDamage();

    float RestoreSpirit(float Amount);

    float CalculateTakeSpiritDamage(float DamageAmount);

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

    void ForceSetMouseSensitivity(float NewSensitivity, float Duration);

    void RestoreOriginalMouseSensitivity();

    void ForceInvertMouse(bool bInvert);

    void ForceInvertMouseTemporary(bool bInvert, float Duration);

    void RestoreMouseInvert();

    void TriggerPanicVoice(float Duration);

};
