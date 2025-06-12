#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCRotationGimmick.h"
#include "LCRotationLuxStatue.generated.h"

class USoundBase;
class USceneComponent;
class UNiagaraSystem;
class UNiagaraComponent;
class UAudioComponent;

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCRotationLuxStatue : public ALCRotationGimmick
{
	GENERATED_BODY()

public:
	ALCRotationLuxStatue();

protected:
	virtual void BeginPlay() override;

	/** 빛 발사를 주기적으로 수행하는 타이머 */
	FTimerHandle LuxEmitTimer;

public:
	/** 빛 발사 여부 (코어 장착 시 true) */
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bIsLuxActive;

	/** 라인트레이스 및 이펙트 기준점 (왼쪽/오른쪽) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	USceneComponent* LightOriginLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	USceneComponent* LightOriginRight;

	/** 빛 발사 거리 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	float LightRange;

	/** 빛 발사 사운드 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	USoundBase* LightActivateSound;

	/** 빛 발사 이펙트 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	UNiagaraSystem* LightEffectTemplate;

	/** 나이아가라 컴포넌트 (왼쪽/오른쪽) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	UNiagaraComponent* LightEffectComponentLeft;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	UNiagaraComponent* LightEffectComponentRight;

	/** 사운드 재생용 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	UAudioComponent* AudioComponent;

	/** 마지막으로 빛을 맞았던 액터 */
	UPROPERTY()
	AActor* LastLitTarget = nullptr;

	/** 디버그용 라인 시각화 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Lux")
	bool bUseDebugLine;

public:
	UFUNCTION(BlueprintCallable, Category = "Lux")
	void ActivateLux();

	UFUNCTION(BlueprintCallable, Category = "Lux")
	void DeactivateLux();

	UFUNCTION()
	void EmitLuxRay();

	UFUNCTION(BlueprintCallable, Category = "Lux")
	bool IsLuxActive() const;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EmitLightEffect(const FVector& End);
	void Multicast_EmitLightEffect_Implementation(const FVector& End);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayLightSound();
	void Multicast_PlayLightSound_Implementation();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};