#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "Perception/PawnSensingComponent.h"
#include "NavigationSystem.h"
#include "LCBossSlenderman.generated.h"

UCLASS()
class LASTCANARY_API ALCBossSlenderman : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossSlenderman();

protected:
    virtual void BeginPlay() override;

    /** ── Echolocation Ping ── */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Echo")
    float PingInterval = 10.f;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Echo")
    float PingRadius = 800.f;

    FTimerHandle PingTimerHandle;
    void EchoPing();

    /** ── Noise Hearing ── */
    UPROPERTY(VisibleAnywhere, Category = "Slenderman|Perception")
    UPawnSensingComponent* PawnSensingComp;

    UPROPERTY(EditAnywhere, Category = "Slenderman|Perception")
    UAnimMontage* ScreamMontage;

    UFUNCTION()
    void OnNoiseHeard(APawn* InstigatorPawn, const FVector& Location, float Volume);

    /** ── Teleport ── */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Teleport")
    float TeleportInterval = 15.f;

    FTimerHandle TeleportTimerHandle;
    void TeleportToRandomLocation();

    /** ── Distortion ── */
    UPROPERTY(EditAnywhere, Category = "Slenderman|Distortion")
    float DistortionInterval = 30.f;

    FTimerHandle DistortionTimerHandle;
    void ExecuteDistortion();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_DistortionEffect();

    /** ── Endless Stalk ── */
    // 60초 후 자동 발동
    FTimerHandle EndlessStalkTimerHandle;

    UPROPERTY(ReplicatedUsing = OnRep_EndlessStalk)
    bool bIsEndlessStalk = false;

    UFUNCTION()
    void OnRep_EndlessStalk();

    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartEndlessStalk();

    void EnterEndlessStalk();

    // Replication
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
