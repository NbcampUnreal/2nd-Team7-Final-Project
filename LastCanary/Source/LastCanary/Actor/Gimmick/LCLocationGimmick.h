#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCLocationGimmick.generated.h"

UCLASS()
class LASTCANARY_API ALCLocationGimmick : public ALCBaseGimmick
{
	GENERATED_BODY()

public:
	ALCLocationGimmick();

protected:
	virtual void BeginPlay() override;

	/** ===== 이동 설정 ===== */

	/** 이동 거리 (기본 Z축, bUseAxis=false일 때 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (EditCondition = "!bUseAxis"))
	float MoveStep;

	/** 이동 벡터 (bUseAxis=true일 때 사용) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (EditCondition = "bUseAxis"))
	FVector MoveVector;

	/** 이동 방향 직접 지정 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement")
	bool bUseAxis;

	/** 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (ClampMin = "1.0"))
	float MoveSpeed;

	/** 현재 이동 인덱스 (이동 횟수) */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick|Movement")
	int32 MoveIndex;

	/** 1번 , 2번 위치 토글 모드 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Toggle")
	bool bUseAlternateToggle;

	/** 이동 시작 위치 */
	FVector InitialLocation;

	/** 이동 목표 위치 */
	FVector TargetLocation;

	/** 원래 위치 (복귀 기준) */
	FVector OriginalLocation;

	/** 위치 토글용 두 번째 위치 (초기 위치 + 이동값) */
	FVector AlternateLocation;

	/** 이동 타이머 핸들 */
	FTimerHandle MovementTimerHandle;

	/** 복귀 타이머 핸들 */
	FTimerHandle ReturnTimerHandle;

	/** 서버 이동 상태 */
	bool bIsMovingServer;

	/** 서버 복귀 상태 */
	bool bIsReturningServer;

	/** ===== 클라이언트 이동 보간 ===== */

	FVector ClientStartLocation;
	FVector ClientTargetLocation;
	float ClientMoveDuration;
	float ClientMoveElapsed;
	FTimerHandle ClientMoveTimer;

	/** ===== 서버 이동 보간 ===== */

	float ServerMoveDuration;
	float ServerMoveElapsed;
	FTimerHandle ServerMoveTimer;

	/** ===== 이동 함수 ===== */

	/** 이동 시작 */
	virtual void StartMovement();

	/** 이동 완료 처리 */
	void CompleteMovement();

	/** 원래 위치로 복귀 시작 */
	void ReturnToInitialLocation();

	/** 복귀 완료 처리 */
	void CompleteReturn();

	/** 클라이언트 보간 이동 시작 */
	void StartClientMovement(const FVector& From, const FVector& To, float Duration);

	/** 클라이언트 보간 이동 진행 */
	void StepClientMovement();

	/** 서버 보간 이동 시작 */
	void StartServerMovement(const FVector& From, const FVector& To, float Duration);

	/** 서버 보간 이동 진행 */
	void StepServerMovement();

	/** 감지된 액터 이동 보간 (서버 전용) */
	void StartServerAttachedMovement(const FVector& DeltaLocation, float Duration);

	/** 감지된 액터 이동 보간 (클라이언트 전용) */
	void StartClientAttachedMovement(const FVector& DeltaLocation, float Duration);

	/** 기믹 사용 가능 여부 */
	virtual bool CanActivate_Implementation() override;

public:
	/** 기믹 활성화 시 이동 시작 */
	virtual void ActivateGimmick_Implementation() override;

	virtual void ReturnToInitialState_Implementation(); 
 
	/** 이동 타이머 캐시 */
	UPROPERTY()
	TMap<AActor*, FTimerHandle> AttachedMovementTimers;

	/** 이동 시작 정보 멀티캐스트 (From, To, Duration) */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartMovement(const FVector& From, const FVector& To, float Duration);
	void Multicast_StartMovement_Implementation(const FVector& From, const FVector& To, float Duration);
};
