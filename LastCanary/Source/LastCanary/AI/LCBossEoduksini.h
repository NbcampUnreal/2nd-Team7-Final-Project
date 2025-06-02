#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "LCBossEoduksini.generated.h"

UCLASS()
class LASTCANARY_API ALCBossEoduksini : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

    friend class ALCBaseBossAIController;

public:
    ALCBossEoduksini();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaSeconds) override;

    /** ── 플레이어 시선 체크 ── */
    bool IsLookedAtByAnyPlayer() const;
    bool IsPlayerLooking(APlayerController* PC) const;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Sight", meta = (ClampMin = "0", ClampMax = "180"))
    float LookAngleDeg = 18.f;

    /** ── 분노(Rage) ── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float MaxRage = 100.f;

    /** 어둠이 적용될 최대 반경 */
    UPROPERTY(EditAnywhere, Category = "Darkness")
    float DarknessRadius = 1000.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float DarknessRage = 60.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageGainPerSec = 8.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Rage")
    float RageLossPerSec = 12.f;

    /** ── 스케일(scale) ── */
    UPROPERTY(ReplicatedUsing = OnRep_CurScale, BlueprintReadOnly, Category = "Boss|Scale")
    float CurScale = 1.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float MinScale = 0.25f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float MaxScale = 1.0f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Scale")
    float ScaleInterpSpeed = 1.5f;

    UFUNCTION()
    void OnRep_CurScale();

    /** 페이드 강도, 시간 (디테일에서 조정 가능) */
    UPROPERTY(EditAnywhere, Category = "Darkness")
    float DarknessFadeAlpha = 0.8f;

    UPROPERTY(EditAnywhere, Category = "Darkness")
    float FadeDuration = 0.5f;

    /** ── Darkness 스킬 ── */
    UPROPERTY(ReplicatedUsing = OnRep_DarknessActive, BlueprintReadOnly, Category = "Boss|Darkness")
    bool bDarknessActive = false;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Darkness")
    float DarknessDuration = 10.f;

    UFUNCTION()
    void OnRep_DarknessActive();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartDarkness();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EndDarkness();

    /** 포스트프로세스나 이펙트를 켤 때 */
    UFUNCTION(BlueprintNativeEvent, Category = "Darkness")
    void BP_StartDarknessEffect();
    virtual void BP_StartDarknessEffect_Implementation();

    /** 포스트프로세스나 이펙트를 끌 때 */
    UFUNCTION(BlueprintNativeEvent, Category = "Darkness")
    void BP_EndDarknessEffect();
    virtual void BP_EndDarknessEffect_Implementation();

    /** 단서 관련 멤버들 */

    /** 할퀸 자국 블루프린트 클래스 참조 */
    UPROPERTY(EditAnywhere, Category = "Clue")
    TSubclassOf<AActor> ScratchMarkClass;

    /** 그림자 얼룩 블루프린트 클래스 참조 */
    UPROPERTY(EditAnywhere, Category = "Clue")
    TSubclassOf<AActor> ShadowStainClass;

    /** 단서를 남길 최소/최대 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Clue", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float ClueSpawnIntervalMin = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Clue", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float ClueSpawnIntervalMax = 12.0f;

    /** 이미 스폰된 단서를 추적하기 위한 배열 (원본 데모에서는 쓸 필요 없지만, 관리용으로 남겨둠) */
    UPROPERTY()
    TArray<AActor*> SpawnedClues;

    /** 클루 스폰 타이머 핸들 */
    FTimerHandle ClueTimerHandle;

    /** 실제 단서를 스폰하는 함수 */
    void SpawnRandomClue();

    FTimerHandle DarknessTimer;

    UPROPERTY()
    TSet<APlayerController*> DarkenedPlayers;

    /** ── 공격(Rage 증감 포함) ── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float RageGain_Normal = 5.f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float RageLoss_Strong = 40.f;

    virtual bool RequestAttack() override;

private:
    void UpdateRageAndScale(float DeltaSeconds);
    void TryTriggerDarkness();
    void EndDarkness();

    void PlayNormalWithRage();
    void PlayStrongWithRage();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
