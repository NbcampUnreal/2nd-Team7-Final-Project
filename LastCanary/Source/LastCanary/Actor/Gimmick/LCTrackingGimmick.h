#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCTrackingGimmick.generated.h"

class ALCTrackingManager;
class UNiagaraComponent;

/**
 *
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCTrackingGimmick : public ALCBaseGimmick
{
	GENERATED_BODY()

public:
	ALCTrackingGimmick();

	/** 타겟 설정 (Manager에서 호출) */
	UFUNCTION(BlueprintCallable, Category = "Tracking")
	void SetTargetActor(AActor* NewTarget);

	/** 타겟 반환 */
	UFUNCTION(BlueprintCallable, Category = "Tracking")
	AActor* GetTargetActor() const;

	/** 감시 시작 (회전 타이머 시작) */
	void StartTracking();

	/** 감시 정지 (타이머 중단) */
	void StopTracking();

	/** 발사 실행 (이펙트 재생) */
	void Fire();

protected:
	virtual void BeginPlay() override;

	/** 현재 감시 타겟 */
	UPROPERTY(Replicated)
	AActor* TargetActor;

	/** 감시탑의 정면 회전 오프셋 (Yaw 보정) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking|Config", meta = (ClampMin = -180.0, ClampMax = 180.0))
	float YawOffset;

	/** 회전 간격 (초 단위) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tracking|Config", meta = (ClampMin = 0.1))
	float TrackingInterval;

	/** 현재 감시 중 여부 */
	UPROPERTY(VisibleInstanceOnly, Category = "Tracking|State")
	bool bIsTracking;

	/** 회전 처리 타이머 */
	FTimerHandle TrackingTimerHandle;

	/** 이펙트 발사 종료용 타이머 */
	FTimerHandle FireTimerHandle;

	/** 주기적으로 타겟을 바라보도록 회전 처리 */
	void RotateToTarget();

	/** 추적 발사 이펙트 - 왼쪽 */
	UPROPERTY(VisibleAnywhere, Category = "Tracking|Effect")
	UNiagaraComponent* TrackingEffectLeft;

	/** 추적 발사 이펙트 - 오른쪽 */
	UPROPERTY(VisibleAnywhere, Category = "Tracking|Effect")
	UNiagaraComponent* TrackingEffectRight;

	/** 이펙트 Yaw 회전 보정값 (좌우 공용) */
	UPROPERTY(EditAnywhere, Category = "Tracking|Effect")
	float EffectYawOffset = 0.f;

	/** 나이아가라 이펙트 발사 동기화 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_FireEffect();
	void Multicast_FireEffect_Implementation();
};