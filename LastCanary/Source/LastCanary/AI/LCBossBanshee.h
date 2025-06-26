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

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Rage")
    float RageDecayPerSecond = 5.f;

    // 포스트프로세스 머티리얼 에셋
    UPROPERTY(EditAnywhere, Category = "Banshee|Debuff")
    UMaterialInterface* FearPostProcessMat;

    // DesperateWail 에서 쓸 변수들
    UPROPERTY(EditAnywhere, Category = "Banshee|Debuff")
    float FearDuration = 4.f;

    /** 포스트프로세스 블렌드 가중치 */
    UPROPERTY(EditAnywhere, Category = "Banshee|Debuff")
    float FearPostProcessWeight = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Banshee|Debuff")
    float SlowDuration = 4.f;

    UPROPERTY(EditAnywhere, Category = "Banshee|Debuff")
    float SlowMultiplier = 0.6f; // 속도 60% 유지

    // RPC: 특정 플레이어 클라이언트에 공포 연출
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_ApplyFear(ACharacter* Target);
    void Multicast_ApplyFear_Implementation(ACharacter* Target);

    /** Desperate Wail 사운드 감쇠 설정 */
    UPROPERTY(EditAnywhere, Category = "Banshee|Sound")
    USoundAttenuation* DesperateWailAttenuation;

    virtual bool RequestAttack(float TargetDistance) override;

private:
    FTimerHandle PingTimerHandle;
    FTimerHandle ShriekTimerHandle;

    bool bCanShriek = true;
    float LastHeardNoiseTime = 0.f;

    /** 최근 EchoSlash 대상 위치 */
    FVector LastPingedLocation;
	
};
