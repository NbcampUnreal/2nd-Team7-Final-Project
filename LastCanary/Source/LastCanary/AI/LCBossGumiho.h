#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "LCBossGumiho.generated.h"

UCLASS()
class LASTCANARY_API ALCBossGumiho : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossGumiho();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── Illusion Dance ──
    /** 소환할 환영 클래스 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    TSubclassOf<AActor> IllusionClass;

    /** 한 번에 생성할 환영 수 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion", meta = (ClampMin = "1", ClampMax = "10"))
    int32 NumIllusions = 3;

    /** 환영 소환 주기(초) */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionInterval = 20.f;

    FTimerHandle IllusionTimerHandle;
    TArray<AActor*> IllusionActors;
    void SpawnIllusions();

    // ── Foxfire Volley ──
    /** 발사할 불꽃 투사체 클래스 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    TSubclassOf<AActor> FoxfireProjectileClass;

    /** 투사체 개수 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire", meta = (ClampMin = "1"))
    int32 FoxfireCount = 5;

    /** 투사 주기(초) */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Foxfire")
    float FoxfireInterval = 15.f;

    FTimerHandle FoxfireTimerHandle;
    void ExecuteFoxfireVolley();

    // ── Illusion Swap ──
    /** 환영 위치 교체 주기(초) */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Illusion")
    float IllusionSwapInterval = 25.f;

    FTimerHandle SwapTimerHandle;
    void PerformIllusionSwap();

    // ── Charm Gaze ──
    /** 매 CharmInterval 초마다 반경 내 플레이어 매혹 */
    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmRadius = 500.f;

    UPROPERTY(EditAnywhere, Category = "Gumiho|Charm")
    float CharmInterval = 10.f;

    FTimerHandle CharmTimerHandle;
    void ExecuteCharmGaze();

    // ── Nine-Tail Burst ──
    bool bHasUsedNineTail = false;
    void ExecuteNineTailBurst();

    // ── Divine Grace ──
    UPROPERTY(ReplicatedUsing = OnRep_DivineGrace, BlueprintReadOnly, Category = "Gumiho|Divine")
    bool bIsDivineGrace = false;

    UFUNCTION()
    void OnRep_DivineGrace();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartDivineGrace();

    // 복제 설정
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
