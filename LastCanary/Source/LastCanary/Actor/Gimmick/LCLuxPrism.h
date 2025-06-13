#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCPushGimmick.h"
#include "Interface/GimmickEffectInterface.h"
#include "LCLuxPrism.generated.h"

class USoundBase;
class UNiagaraSystem;
class UNiagaraComponent;
class UAudioComponent;
class USceneComponent;
/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCLuxPrism : public ALCPushGimmick, public IGimmickEffectInterface
{
	GENERATED_BODY()
	
public:
	ALCLuxPrism();

protected:
	virtual void BeginPlay() override;

	/** 현재 빛을 받고 있는 상태 여부 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bIsLuxReceived;

	/** 광선 발사 위치 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	USceneComponent* EmitOrigin;

	/** 발사 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	float LightRange;

	/** 디버그 라인 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bUseDebugLine;

	/** 발사 간격 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	float EmitInterval;

	/** 발사 이펙트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	UNiagaraSystem* EmitEffect;

	/** 발사 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	USoundBase* EmitSound;

	/** 나이아가라 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	UNiagaraComponent* NiagaraComponent;

	/** 오디오 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	UAudioComponent* AudioComponent;

	/** 발사 타이머 핸들 */
	FTimerHandle EmitTimerHandle;

protected:
	/** 주기적으로 빛을 발사 */
	void EmitLux();

	/** 발사 시작 */
	void StartEmitLux();

	/** 발사 중지 */
	void StopEmitLux();

public:
	virtual void ActivateGimmick_Implementation() override;
	virtual void DeactivateGimmick_Implementation() override;

	/** 빛을 받음 (IGimmickEffectInterface 구현) */
	virtual void TriggerEffect_Implementation() override;

	/** 빛 종료 (IGimmickEffectInterface 구현) */
	virtual void StopEffect_Implementation() override;

	/** 멀티캐스트: 발사 시작 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartEmitLux();
	void Multicast_StartEmitLux_Implementation();

	/** 멀티캐스트: 발사 종료 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StopEmitLux();
	void Multicast_StopEmitLux_Implementation();
};