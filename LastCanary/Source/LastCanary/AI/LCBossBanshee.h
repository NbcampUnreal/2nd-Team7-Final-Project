#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "NiagaraSystem.h"
#include "AI/Summon/CloneMinion.h"
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
    virtual void UpdateBlackboardValues() override;

    UFUNCTION(BlueprintCallable, Category = "Banshee|Hearing")
    void OnHeardNoise(const FVector& NoiseLocation);

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    void HandleRehide(ACharacter* Char);
    void AddRage(float Amount);
    void DecayRage(float DeltaTime);

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

    /** EcholocationPing */
    void EcholocationPing();

    /** 멀티캐스트용: 모든 클라이언트에서 Echo 사운드를 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayEcholocationSound(const FVector& Location);

    /** Echo 사운드용 감쇠 세팅 */
    UPROPERTY(EditAnywhere, Category = "Banshee|Audio")
    USoundAttenuation * EcholocationAttenuation;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float PingInterval = 30.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float PingRadius = 2000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float RevealDuration = 4.f;

    /** Sonic Shriek*/

    /** Sonic Shriek용 감쇠 세팅 */
    UPROPERTY(EditAnywhere, Category = "Banshee|Audio")
    USoundAttenuation* SonicShriekAttenuation;

    /** 모든 클라이언트에서 Sonic Shriek 사운드 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlaySonicShriekSound(const FVector& Location);

    void ResetShriek();

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekDamage = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekRadius = 3000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekCooldown = 10.f;

    /** Wail: 근접 범위에 울음파를 발사해 넉백 및 공포 디버프 */
    UFUNCTION()
    void Wail();

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailDamage = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailRange = 400.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float EchoSlashCooldown = 10.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailCooldown = 6.f;

    /** Wail 사운드용 감쇠 세팅 */
    UPROPERTY(EditAnywhere, Category = "Banshee|Audio")
    USoundAttenuation* AttackAttenuation;

    /** 멀티캐스트로 모든 클라이언트에서 Wail 사운드를 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayWailSound(const FVector& Location);

    /** Echo Slash: 핑된 위치로 순간이동 후 광역 베기 */
    void EchoSlash();

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float EchoSlashDamage = 10.f;

    /** Desperate Wail: 광역 공포 울음 + 슬로우 디버프 */
    void DesperateWail();

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float DesperateWailDamage = 40.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Rage")
    float DesperateWailRageThreshold = 80.f;

    /** Desperate Wail 효과를 모든 클라이언트에서 재생 */
    UFUNCTION(NetMulticast, Unreliable)
    void Multicast_PlayDesperateWailEffects(const FVector& Origin);

    /** 분신 소환 */
    void SpawnBansheeClones();

    // 분신 클래스와 생성 반경, 개수
    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Clone")
    TSubclassOf<ACloneMinion> CloneClass;


    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Special")
    float CloneSpawnRadius = 600.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Special")
    int32 CloneCount = 4;

    float LastEchoSlashTime = 0.f;
    float LastWailTime = 0.f;
    float LastBasicTime = 0.f;

    /** --- 공격 관련 수치 설정 --- */

    // 버서크 시 적용할 멀티플라이어
    UPROPERTY(EditAnywhere, Category = "Banshee|Berserk")
    float BerserkPingRadiusMultiplier = 1.5f;

    UPROPERTY(EditAnywhere, Category = "Banshee|Berserk")
    float BerserkShriekCooldownMultiplier = 0.5f;

    UPROPERTY(EditAnywhere, Category = "Banshee|Berserk")
    float BerserkWailRangeMultiplier = 1.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float BasicAttackCooldown = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float EchoSlashWeight = 2.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float WailWeight = 3.f;

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Combat")
    float BasicAttackWeight = 5.f;

    /** Ping 당 얻는 Rage */
    UPROPERTY(EditAnywhere, Category = "Banshee|Rage")
    float PingRageGain = 2.f;

    /** Ping 당 잃는 Rage (적을 찾지 못했을 때) */
    UPROPERTY(EditAnywhere, Category = "Banshee|Rage")
    float PingRageLoss = 5.f;

    /** 소음 감지 시 얻는 Rage 양 */
    UPROPERTY(EditAnywhere, Category = "Banshee|Rage")
    float NoiseRageGain = 1.f;     // 소음감지 100번시 광폭

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Rage")
    float RageDecayPerSecond = 0.1f;

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

    virtual bool RequestAttack(float TargetDistance) override;

private:
    FTimerHandle PingTimerHandle;
    FTimerHandle ShriekTimerHandle;

    bool bCanShriek = true;
    bool bHasUsedDesperateWail = false;
    float LastHeardNoiseTime = 0.f;

    // 원래 값을 저장해 둘 변수들
    float PrevPingRadius;
    float PrevShriekCooldown;
    float PrevWailRange;

    /** 최근 EchoSlash 대상 위치 */
    FVector LastPingedLocation;
	
};
