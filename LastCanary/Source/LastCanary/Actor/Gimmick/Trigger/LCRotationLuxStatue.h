#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCRotationGimmick.h"
#include "LCRotationLuxStatue.generated.h"

class USoundBase;
class USceneComponent;

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

	// 주 라인트레이스 기준점
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lux")
	USceneComponent* LightOriginMain;

	// 시각 이펙트용 (왼쪽/오른쪽)
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

public:
	/** 빛 코어 장착 시 Lux 상태 활성화 */
	UFUNCTION(BlueprintCallable, Category = "Lux")
	void ActivateLux();

	/** 서버: 빛 발사 및 라인트레이스 */
	UFUNCTION()
	void EmitLuxRay();

	/** Lux 상태 확인 함수 */
	UFUNCTION(BlueprintCallable, Category = "Lux")
	bool IsLuxActive() const;

	/** Lux 비활성화 함수 */
	UFUNCTION(BlueprintCallable, Category = "Lux")
	void DeactivateLux();

	/** 빛 이펙트 연출 (보간 포함) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_EmitLightEffect(const FVector& Start, const FVector& End);
	void Multicast_EmitLightEffect_Implementation(const FVector& Start, const FVector& End);

	/** 클라이언트: 빛 사운드 출력 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_PlayLightSound();
	void Multicast_PlayLightSound_Implementation();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};