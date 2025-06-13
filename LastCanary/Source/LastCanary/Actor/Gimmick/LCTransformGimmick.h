#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCTransformGimmick.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCTransformGimmick : public ALCBaseGimmick
{
	GENERATED_BODY()
public:
	ALCTransformGimmick();

protected:
	virtual void BeginPlay() override;
	virtual void ActivateGimmick_Implementation() override;
	virtual bool CanActivate_Implementation() override;

public:

	/** 1번 ↔ 2번 위치 토글 모드 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Toggle")
	bool bUseAlternateToggle;

	/** 위치 토글용 두 번째 위치 */
	FVector AlternateLocation;

	/** 회전 토글용 두 번째 회전값 */
	FRotator AlternateRotation;

	virtual void ReturnToInitialState_Implementation() override;

#pragma region Movement Variables

	/** 이동 방향 벡터 (절대 방향) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement")
	FVector MoveVector;

	/** 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (ClampMin = "1.0"))
	float MoveDuration;

	/** 복귀 이동 속도 (cm/s) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Movement", meta = (ClampMin = "1.0"))
	float ReturnMoveDuration;

	/** 이동 인덱스 (몇 번 이동했는지) */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick|Movement")
	int32 MoveIndex;

	/** 시작 위치 */
	FVector InitialLocation;

	/** 목표 위치 */
	FVector TargetLocation;

	/** 원래 위치 (복귀용) */
	FVector OriginalLocation;

	/** 서버 이동 중 여부 */
	bool bIsMovingServer;

	/** 서버 복귀 이동 중 여부 */
	bool bIsReturningServer;

	/** 이동 타이머 */
	FTimerHandle MovementTimerHandle;

	/** 복귀 이동 타이머 */
	FTimerHandle ReturnMoveTimerHandle;

	/** 클라이언트 보간 이동용 */
	FVector ClientStartLocation;
	FVector ClientTargetLocation;
	float ClientMoveDuration;
	float ClientMoveElapsed;
	FTimerHandle ClientMoveTimer;

	/** 서버 보간 이동용 */
	float ServerMoveDuration;
	float ServerMoveElapsed;
	FTimerHandle ServerMoveTimer;

	/** 외부에서 이동 목표 위치를 직접 지정할 때 사용 (PushGimmick 전용) */
	virtual void StartMovementToTarget(const FVector& NewTarget);

#pragma endregion

#pragma region Rotation Variables

	/** 회전 축 (Pitch, Yaw, Roll) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation")
	FVector RotationAxis;

	/** 회전 속도 (deg/sec) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation", meta = (ClampMin = "1.0"))
	float RotationDuration;

	/** 복귀 회전 속도 (deg/sec) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Rotation", meta = (ClampMin = "1.0"))
	float ReturnRotationDuration;

	/** 회전 인덱스 */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick|Rotation")
	int32 RotationIndex;

	/** 최초 회전값 */
	FRotator OriginalRotation;

	/** 회전 시작값 */
	FRotator InitialRotation;

	/** 회전 목표값 */
	FRotator TargetRotation;

	/** 누적 회전량 (Yaw 기준으로 누적) */
	FRotator AccumulatedDeltaRotation;

	/** 서버 회전 중 여부 */
	bool bIsRotatingServer;

	/** 서버 복귀 회전 중 여부 */
	bool bIsReturningRotationServer;

	/** 회전 타이머 */
	FTimerHandle RotationTimerHandle;

	/** 복귀 회전 타이머 */
	FTimerHandle ReturnRotationTimerHandle;

	/** 클라이언트 보간 회전용 */
	FRotator AccumulatedRotation;
	FRotator ClientStartRotation;
	FRotator ClientTargetRotation;
	float ClientRotationDuration;
	float ClientRotationElapsed;
	FTimerHandle ClientRotationTimer;

	/** 서버 보간 회전용 */
	FRotator ServerStartRotation;
	FRotator ServerTargetRotation;
	float ServerRotationDuration;
	float ServerRotationElapsed;
	FTimerHandle ServerRotationTimer;

#pragma endregion

#pragma region Movement Functions

	/** 이동 시작 (서버) */
	virtual void StartMovement();

	/** 이동 완료 */
	virtual void CompleteMovement();

	/** 복귀 이동 시작 */
	void ReturnToInitialLocation();

	/** 복귀 이동 완료 */
	void CompleteReturn();

	/** 서버 보간 이동 시작 */
	void StartServerMovement(const FVector& From, const FVector& To, float Duration);

	/** 서버 보간 이동 실행 */
	void StepServerMovement();

	/** 클라이언트 보간 이동 시작 */
	void StartClientMovement(const FVector& From, const FVector& To, float Duration);

	/** 클라이언트 보간 이동 실행 */
	void StepClientMovement();

	/** 감지된 액터 클라이언트 회전 보간 */
	void StartClientAttachedRotation(const FRotator& DeltaRot, float Duration);

	/** 감지된 액터 서버 회전 보간 */
	void StartServerAttachedRotation(const FRotator& DeltaRot, float Duration);

	/** 감지된 액터 이동 보간 (서버 전용) */
	void StartServerAttachedMovement(const FVector& DeltaLocation, float Duration);

	/** 감지된 액터 이동 보간 (클라이언트 전용) */
	void StartClientAttachedMovement(const FVector& DeltaLocation, float Duration);

	/** 이동 타이머 캐시 */
	UPROPERTY()
	TMap<AActor*, FTimerHandle> AttachedMovementTimers;

	/** 멀티캐스트 이동 시작 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartMovement(const FVector& From, const FVector& To, float Duration);
	void Multicast_StartMovement_Implementation(const FVector& From, const FVector& To, float Duration);


#pragma endregion

#pragma region Rotation Functions

	/** 회전 시작 (서버) */
	void StartRotation();

	/** 회전 완료 */
	void CompleteRotation();

	/** 복귀 회전 시작 */
	void ReturnToInitialRotation();

	/** 복귀 회전 완료 */
	void CompleteRotationReturn();

	/** 서버 보간 회전 시작 */
	void StartServerRotation(const FRotator& From, const FRotator& To, float Duration);

	/** 서버 보간 회전 실행 */
	void StepServerRotation();

	/** 클라이언트 보간 회전 시작 */
	void StartClientRotation(const FRotator& From, const FRotator& To, float Duration);

	/** 클라이언트 보간 회전 실행 */
	void StepClientRotation();

	/** 회전 타이머 캐시 */
	UPROPERTY()
	TMap<AActor*, FTimerHandle> AttachedRotationTimers;

	/** 멀티캐스트 회전 시작 */
	UFUNCTION(NetMulticast, Reliable)
	void Multicast_StartRotation(const FRotator& From, const FRotator& To, float Duration);
	void Multicast_StartRotation_Implementation(const FRotator& From, const FRotator& To, float Duration);

#pragma endregion
};