#pragma once

#include "CoreMinimal.h"
#include "AI/BaseMonsterCharacter.h"
#include "LCBossMonsterCharacter.generated.h"

class UNiagaraSystem;
class UTimelineComponent;

UCLASS()
class LASTCANARY_API ALCBossMonsterCharacter : public ABaseMonsterCharacter
{
	GENERATED_BODY()

	friend class ALCBossAIController;

public:
	ALCBossMonsterCharacter();

	/* ───── 설정값 (EditDefaultsOnly) ───── */
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Sight")   float LookAngleDeg = 18.f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Rage")    float MaxRage = 100.f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Rage")    float DarknessRage = 60.f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Rage")    float RageGainPerSec = 8.f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Rage")    float RageLossPerSec = 12.f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Scale")   float MinScale = 0.25f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Scale")   float MaxScale = 1.0f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Scale")   float ScaleInterpSpeed = 1.5f;
	UPROPERTY(EditDefaultsOnly, Category = "Eoduksuni|Darkness")float DarknessDuration = 10.f;

	/* ───── 실시간 상태 (Replicated) ───── */
	UPROPERTY(ReplicatedUsing = OnRep_Rage, BlueprintReadOnly, Category = "Eoduksuni|Rage")   float Rage = 0.f;
	UPROPERTY(ReplicatedUsing = OnRep_Scale, BlueprintReadOnly, Category = "Eoduksuni|Scale")  float CurScale = 1.f;
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Eoduksuni|Darkness") bool bDarknessActive = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/* ---- 내부 처리 ---- */
	bool IsLookedAtByAnyPlayer() const;
	bool IsPlayerLooking(class APlayerController* PC) const;

	void UpdateRage(float DeltaSeconds, bool bLooked);
	void UpdateScale(float DeltaSeconds, bool bLooked);

	void TryTriggerDarkness();
	void EndDarkness();

	/* ---- RepNotify ---- */
	UFUNCTION() void OnRep_Rage();
	UFUNCTION() void OnRep_Scale();

	/* ---- 네트워크 ---- */
	UFUNCTION(NetMulticast, Reliable) void Multicast_StartDarkness();
	UFUNCTION(NetMulticast, Reliable) void Multicast_EndDarkness();

	/* ---- 블루프린트 훅 ---- */
	UFUNCTION(BlueprintImplementableEvent, Category = "Eoduksuni|Darkness")
	void BP_StartDarknessEffect();   // 화면 PostProcess / HUD 등
	UFUNCTION(BlueprintImplementableEvent, Category = "Eoduksuni|Darkness")
	void BP_EndDarknessEffect();

	FTimerHandle DarknessTimer;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};
