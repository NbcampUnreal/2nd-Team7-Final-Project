#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "TimerManager.h"
#include "LCRotationGimmick.generated.h"

UCLASS()
class LASTCANARY_API ALCRotationGimmick : public ALCBaseGimmick
{
	GENERATED_BODY()

public:
	ALCRotationGimmick();

protected:
	virtual void BeginPlay() override;

	/** ===== 회전 설정 ===== */

	/** 회전 각도 (Yaw 중심, bUseAxis=false일 때 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation", meta = (EditCondition = "!bUseAxis"))
	float RotationStep;

	/** 회전 축 (Pitch/Yaw/Roll, bUseAxis=true일 때 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation", meta = (EditCondition = "bUseAxis"))
	FVector RotationAxis;

	/** 회전 축 직접 지정 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation")
	bool bUseAxis;

	/** 회전 속도 (deg/sec) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation", meta = (ClampMin = "1.0"))
	float RotationSpeed;

	/** 현재 회전 인덱스 (회전 횟수) */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick|Rotation")
	int32 RotationIndex;

	/** 회전 토글 모드 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Toggle")
	bool bUseAlternateToggle;

	/** 회전 시작값 */
	FRotator InitialRotation;

	/** 회전 목표값 */
	FRotator TargetRotation;

	/** 초기 회전값 기억 */
	FRotator OriginalRotation;

	/** 회전 토글용 두 번째 회전값 */
	FRotator AlternateRotation;

	/** 회전 타이머 핸들 */
	FTimerHandle RotationTimerHandle;

	/** 복귀 타이머 핸들 */
	FTimerHandle ReturnTimerHandle;

	/** 서버 회전 상태 */
	bool bIsRotatingServer;

	/** 서버 복귀 상태 */
	bool bIsReturningServer;

	/** ===== 클라이언트 회전 보간 ===== */

	/** 보간 시작 회전값 */
	FRotator ClientStartRotation;

	/** 보간 목표 회전값 */
	FRotator ClientTargetRotation;

	/** 보간 시간 및 진행률 */
	float ClientRotationDuration;
	float ClientRotationElapsed;

	/** 클라 회전 보간용 타이머 */
	FTimerHandle ClientRotationTimer;

	/** 서버 보간 회전 타이머 */
	float ServerRotationDuration;
	float ServerRotationElapsed;
	FTimerHandle ServerRotationTimer;

	/** 부착된 액터의 회전 보간 타이머 캐시 (클라이언트용) */
	UPROPERTY()
	TMap<AActor*, FTimerHandle> AttachedRotationTimers;

	/** ===== 내부 회전 함수 ===== */

	/** 회전 시작 */
	virtual void StartRotation();

	/** 회전 완료 처리 */
	void CompleteRotation();

	/** 원래 위치로 복귀 시작 */
	void ReturnToInitialRotation();

	/** 복귀 완료 처리 */
	void CompleteReturn();

	/** 클라이언트 보간 회전 시작 */
	void StartClientRotation(const FRotator& From, const FRotator& To, float Duration);

	/** 클라이언트 보간 회전 진행 */
	void StepClientRotation();

	/** 서버 보간 회전 시작 */
	void StartServerRotation(const FRotator& From, const FRotator& To, float Duration);

	/** 서버 보간 회전 진행 */
	void StepServerRotation();

	/** 기믹 사용 가능 여부 */
	virtual bool CanActivate_Implementation() override;

	/** 기믹이 현재 회전/이동 중인지 여부 반환 */
	virtual bool IsGimmickBusy_Implementation() override;

	/** 감지된 액터 클라이언트 회전 보간 */
	void StartClientAttachedRotation(const FRotator& DeltaRot, float Duration);

	/** 감지된 액터 서버 회전 보간 */
	void StartServerAttachedRotation(const FRotator& DeltaRot, float Duration);

public:
	/** 기믹 활성화 시 회전 시작 */
	virtual void ActivateGimmick_Implementation() override;

	/** 기믹 상태 복귀 처리 오버라이드 */
	virtual void ReturnToInitialState_Implementation() override;

	/** 회전 시작 정보 멀티캐스트 (From, To, Duration) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartRotation(const FRotator& From, const FRotator& To, float Duration);
	void Multicast_StartRotation_Implementation(const FRotator& From, const FRotator& To, float Duration);
};
