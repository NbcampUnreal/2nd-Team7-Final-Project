#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h" 
#include "BaseBossMonsterCharacter.generated.h"

UCLASS()
class LASTCANARY_API ABaseBossMonsterCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    ABaseBossMonsterCharacter();

    /** ─── 공격 설정 ─── */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    UAnimMontage* NormalAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    UAnimMontage* StrongAttackMontage;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack", meta = (ClampMin = "0.0", ClampMax = "1.0"))
    float StrongAttackChance = 0.3f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float NormalAttackCooldown = 1.2f;

    UPROPERTY(EditDefaultsOnly, Category = "Boss|Attack")
    float StrongAttackCooldown = 6.0f;

    /** 일반 공격 대미지 */
    UPROPERTY(EditAnywhere, Category = "Attack")
    float NormalAttackDamage = 20.f;

    /** 강공격 대미지 */
    UPROPERTY(EditAnywhere, Category = "Attack")
    float StrongAttackDamage = 50.f;

    /** 공격 범위 (반경) */
    UPROPERTY(EditAnywhere, Category = "Attack")
    float AttackRange = 200.f;

    /** 현재 보유한 Rage 값 (복제) */
    UPROPERTY(Replicated, BlueprintReadOnly, Category = "Boss|Attack")
    float Rage = 0.f;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nav")
    float NavGenerationradius;

    UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Nav")
    float NavRemovalradius;

    /** 공격 요청 진입 함수 (성공 시 true 반환) */
    UFUNCTION(BlueprintCallable, Category = "Boss|Attack")
    virtual bool RequestAttack();

    /** 외부(예: 게임 모드, 코어 Actor 등)에서 호출해 보스를 Berserk 상태로 전환 */
    UFUNCTION(BlueprintCallable, Category = "Boss|Berserk")
    void EnterBerserkState();

protected:
    /** 실제 공격 처리(Montage 재생) */
    virtual void PlayNormalAttack();
    virtual void PlayStrongAttack();

    /** 몽타주 종료 콜백(파생 클래스에서 override 가능) */
    UFUNCTION()
    void OnAttackMontageEnded(UAnimMontage* Montage, bool bInterrupted);

    float LastNormalTime = -FLT_MAX;
    float LastStrongTime = -FLT_MAX;

    /** ── 광폭화(Berserk) 상태 ── */

    /** 현재 Berserk 활성 여부 (Replicated) */
    UPROPERTY(ReplicatedUsing = OnRep_IsBerserk, BlueprintReadWrite, EditAnywhere, Category = "Boss|Berserk") // 임시로 ReadWrite
    bool bIsBerserk = false;

    /** Berserk 상태에서 Rage 증가 속도 배수 */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float RageGainMultiplier_Berserk = 2.0f;

    /** Berserk 상태에서 데미지 배수 (공격 로직에서 참조) */
    UPROPERTY(EditDefaultsOnly, Category = "Boss|Berserk")
    float DamageMultiplier_Berserk = 1.5f;

    /** Replication 후 처리용 OnRep 함수 */
    UFUNCTION()
    void OnRep_IsBerserk();

    /** Berserk 시작을 클라이언트 전체에 알리는 멀티캐스트 RPC */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartBerserk();

    /** Berserk 종료를 클라이언트 전체에 알리는 멀티캐스트 RPC */
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_EndBerserk();

    /** 실제 Berserk 상태 진입 로직 */
    void StartBerserk();

    /** Berserk 종료 처리 로직 */
    void EndBerserk();

    /** Rage 업데이트 시 Berserk 배수를 적용하고 싶으면 이 함수를 오버라이드하여 사용 */
    virtual void UpdateRage(float DeltaSeconds);

    /** ── 단서(Clue) 관련 멤버들 ── */

/** 단서를 남길 최소/최대 간격 (초) */
    UPROPERTY(EditAnywhere, Category = "Clue", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float ClueSpawnIntervalMin = 5.0f;

    UPROPERTY(EditAnywhere, Category = "Clue", meta = (ClampMin = "1.0", ClampMax = "30.0"))
    float ClueSpawnIntervalMax = 12.0f;

    /** 단서로 스폰할 액터 클래스들을 배열로 선언 */
    UPROPERTY(EditAnywhere, Category = "Clue", meta = (EditFixedOrder))
    TArray<TSubclassOf<AActor>> ClueClasses;

    /** SpawnRandomClue용: 남은 단서 클래스들만 담아두는 배열 */
    UPROPERTY()
    TArray<TSubclassOf<AActor>> RemainingClueClasses;

    /** 클루 스폰 타이머 핸들 */
    FTimerHandle ClueTimerHandle;

    /** 실제 단서를 스폰하는 함수 */
    void SpawnRandomClue();

    // Replication 설정
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
    /** 범위 내 플레이어에게 대미지 적용 */
    void DealDamageInRange(float DamageAmount);
};
