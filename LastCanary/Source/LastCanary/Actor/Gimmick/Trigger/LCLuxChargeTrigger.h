#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCTransformGimmick.h"
#include "Interface/GimmickEffectInterface.h"
#include "Components/PointLightComponent.h"
#include "LCLuxChargeTrigger.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCLuxChargeTrigger : public ALCTransformGimmick, public IGimmickEffectInterface
{
	GENERATED_BODY()

public:
	ALCLuxChargeTrigger();

protected:
	virtual void BeginPlay() override;

	/** 현재 충전 상태 (0.0 ~ 1.0) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	float CurrentCharge;

	/** 빛 수신 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bIsReceivingLux;

	/** 충전 완료 후 발동 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bChargeCompleted;

	/** 충전 속도 (1초에 얼마나 차는지, 1.0이면 1초만에 완충) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux", meta = (ClampMin = 0.01))
	float ChargeRate;

	/** 방전 속도 (1초에 얼마나 줄어드는지, 1.0이면 1초만에 완전 방전) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux", meta = (ClampMin = 0.01))
	float DecayRate;

	/** 충전 상태 갱신 간격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	float TickInterval;

	/** 시각 효과용 라이트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	UPointLightComponent* ChargeLight;

	/** 최대 밝기 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	float MaxLightIntensity;

	/** 타이머 핸들 */
	FTimerHandle ChargeTimerHandle;

	/** 충전 로직 처리 */
	void UpdateCharge();

public:
	/** 빛 수신 시작 */
	virtual void TriggerEffect_Implementation() override;

	/** 빛 수신 종료 */
	virtual void StopEffect_Implementation() override;

	virtual void ActivateGimmick_Implementation() override;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_TriggerEffect();
	void Multicast_TriggerEffect_Implementation();
};
