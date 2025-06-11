#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "LCBossLich.generated.h"

UCLASS()
class LASTCANARY_API ALCBossLich : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossLich();

protected:
    virtual void BeginPlay() override;
    virtual void UpdateRage(float DeltaSeconds) override;

    /** ── 언데드 소환 ── */
    /** 소환할 언데드 클래스 목록 */
    UPROPERTY(EditAnywhere, Category = "Lich|Undead")
    TArray<TSubclassOf<APawn>> UndeadClasses;

    /** 소환 주기 */
    UPROPERTY(EditAnywhere, Category = "Lich|Undead", meta = (ClampMin = "5.0"))
    float UndeadSpawnInterval = 20.f;

    /** 소환 타이머 핸들 */
    FTimerHandle UndeadSpawnTimerHandle;

    /** 언데드 소환 함수 */
    void SpawnUndeadMinion();

    /** ── 마나 파동 ── */
    /** 마나 파동 데미지 */
    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseDamage = 15.f;

    /** 마나 파동 반경 */
    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse")
    float ManaPulseRadius = 600.f;

    /** 마나 파동 주기 */
    UPROPERTY(EditAnywhere, Category = "Lich|ManaPulse", meta = (ClampMin = "1.0"))
    float ManaPulseInterval = 25.f;

    /** 마나 파동 타이머 */
    FTimerHandle ManaPulseTimerHandle;

    /** 마나 파동 실행 함수 */
    void ExecuteManaPulse();

    /** ── 소환된 Phylactery ── */
    /** Phylactery 액터 레퍼런스 (월드에 배치하거나 스폰) */
    UPROPERTY(EditAnywhere, Category = "Lich|Phylactery")
    AActor* PhylacteryActor;

    /** Phylactery 파괴 체크 */
    void CheckPhylactery();

    /** Berserk 진입 조건 재정의 (Phylactery 파괴 시 즉시) */
    virtual void OnRep_IsBerserk() override;

    virtual bool RequestAttack() override;
	
};
