#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "LCBossBanshee.generated.h"

UCLASS()
class LASTCANARY_API ALCBossBanshee : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossBanshee();

protected:
    virtual void BeginPlay() override;

    /** 반향 탐지 핑: 일정 반경 내 플레이어를 잠시 드러내는 기능 **/
    UFUNCTION()
    void EcholocationPing();

    /** Berserk 상태일 때 반향 추가 효과 */
    void BerserkExtraEcho();

    /** AIController나 Behavior Tree가 소리를 감지했을 때 호출됨 **/
    UFUNCTION(BlueprintCallable, Category = "Banshee|Hearing")
    void OnHeardNoise(const FVector& NoiseLocation);

    /** 울부짖기 재사용 대기 상태를 초기화 **/
    void ResetShriek();

    /** --- 반향 탐지 설정 --- **/
    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float PingInterval = 8.f;     // 핑 주기 (초)

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float PingRadius = 2000.f;    // 핑 반경

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Echolocation")
    float RevealDuration = 4.f;   // 드러내는 시간 (초)

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sound")
    USoundBase* EcholocationSound = nullptr; // 반향 소리

    /** --- 소닉 울부짖기 설정 --- **/
    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekDamage = 75.f;     // 울부짖기 데미지

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekRadius = 800.f;    // 울부짖기 반경

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sonic")
    float ShriekCooldown = 6.f;    // 울부짖기 재사용 대기 시간

    UPROPERTY(EditDefaultsOnly, Category = "Banshee|Sound")
    USoundBase* SonicShriekSound = nullptr;  // 울부짖는 소리

private:
    /** 타이머 핸들 **/
    FTimerHandle PingTimerHandle;
    FTimerHandle ShriekTimerHandle;

    /** 울부짖기 가능 여부 **/
    bool bCanShriek = true;
	
};
