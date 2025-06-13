#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
#include "LCBossBanshee.generated.h"

UCLASS()
class LASTCANARY_API ALCBossBanshee : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossBanshee();
    virtual void EnterBerserkState() override;
    virtual void StartBerserk() override;
    virtual void StartBerserk(float Duration) override;
    virtual void EndBerserk() override;

    UFUNCTION(BlueprintCallable, Category = "Banshee|Hearing")
    void OnHeardNoise(const FVector& NoiseLocation);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    virtual void OnRep_IsBerserk() override;

    void EcholocationPing();
    void HandleRehide(ACharacter* Char);



    void ResetShriek();
    void AddRage(float Amount);
    void DecayRage(float DeltaTime);

    UFUNCTION(NetMulticast, Reliable)
    void MulticastActivateBerserkEffects();

    /** Wail: 근접 범위에 울음파를 발사해 넉백 및 공포 디버프 */
    UFUNCTION()
    void Wail();

    /** Echo Slash: 핑된 위치로 순간이동 후 광역 베기 */
    void EchoSlash();

    /** Desperate Wail: 광역 공포 울음 + 슬로우 디버프 */
    void DesperateWail();

    /** 분신 소환 */
    void SpawnBansheeClones();

    /** --- 사운드 & 이펙트 --- */
    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundBase* WailSound = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Effect")
    UNiagaraSystem* WailFX = nullptr;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundBase* EcholocationSound;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundBase* SonicShriekSound;

    UPROPERTY(EditDefaultsOnly, Category = "Sound")
    USoundBase* DesperateWailSound;

    UPROPERTY(EditDefaultsOnly, Category = "|FX")
    UNiagaraSystem* DesperateWailFX;

    UPROPERTY(EditDefaultsOnly, Category = "Effect")
    UNiagaraSystem* BerserkEffectFX;

    UPROPERTY(EditDefaultsOnly, Category = "Effect")
    UNiagaraSystem* EchoSlashFX;

    UPROPERTY(EditDefaultsOnly, Category = "Effect")
    UNiagaraSystem* CloneSpawnFX;

    /** --- 수치 설정 --- */
    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float PingInterval = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float PingRadius = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float RevealDuration = 4.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekDamage = 75.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekRadius = 800.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekCooldown = 6.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Special")
    float CloneSpawnRadius = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Special")
    int32 CloneCount = 4;

    float LastEchoSlashTime = 0.f;
    float LastWailTime = 0.f;
    float LastBasicTime = 0.f;

    bool bHasUsedDesperateWail = false;

    /** --- 공격 관련 수치 설정 --- */

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailRange = 400.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float EchoSlashCooldown = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailCooldown = 6.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float BasicAttackCooldown = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float EchoSlashWeight = 2.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailWeight = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float BasicAttackWeight = 5.f;

    /** --- Rage --- */

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Rage")
    float MaxRage = 100.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Rage")
    float RageDecayPerSecond = 5.f;

    virtual bool RequestAttack(float TargetDistance) override;

private:
    FTimerHandle PingTimerHandle;
    FTimerHandle ShriekTimerHandle;

    bool bCanShriek = true;
    float LastHeardNoiseTime = 0.f;

    /** 최근 EchoSlash 대상 위치 */
    FVector LastPingedLocation;
	
};
