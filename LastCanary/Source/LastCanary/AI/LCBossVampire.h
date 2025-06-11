#pragma once

#include "CoreMinimal.h"
#include "AI/BaseBossMonsterCharacter.h"
#include "LCBossVampire.generated.h"

UCLASS()
class LASTCANARY_API ALCBossVampire : public ABaseBossMonsterCharacter
{
	GENERATED_BODY()

public:
    ALCBossVampire();

protected:
    virtual void BeginPlay() override;

    /** ── Blood Drain (영혼 흡수 힐) ── */
    // OnAttackMontageEnded에서 처리

    /** ── Bat Swarm ── */
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat")
    TSubclassOf<AActor> BatSwarmClass;
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat", meta = (ClampMin = "1"))
    int32 BatCount = 10;
    UPROPERTY(EditAnywhere, Category = "Vampire|Bat")
    float BatSwarmInterval = 30.f;
    FTimerHandle BatSwarmTimerHandle;
    void ExecuteBatSwarm();

    /** ── Mist Form ── */
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistDuration = 5.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|Mist")
    float MistCooldown = 20.f;
    FTimerHandle MistCooldownHandle;
    FTimerHandle MistResetTimerHandle;
    bool bCanUseMist = true;
    UPROPERTY(ReplicatedUsing = OnRep_MistForm, BlueprintReadOnly, Category = "Vampire|Mist")
    bool bIsMistForm = false;
    UFUNCTION()
    void OnRep_MistForm();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartMistForm();
    void EnterMistForm();
    void EndMistForm();
    void ResetMist();

    /** ── Nightmare Gaze ── */
    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeRadius = 600.f;
    UPROPERTY(EditAnywhere, Category = "Vampire|Gaze")
    float GazeInterval = 15.f;
    FTimerHandle GazeTimerHandle;
    void ExecuteNightmareGaze();

    /** ── Eternal Bloodlust ── */
    UPROPERTY(ReplicatedUsing = OnRep_Bloodlust, BlueprintReadOnly, Category = "Vampire|Bloodlust")
    bool bIsBloodlust = false;
    UFUNCTION()
    void OnRep_Bloodlust();
    UFUNCTION(NetMulticast, Reliable)
    void Multicast_StartBloodlust();

    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
