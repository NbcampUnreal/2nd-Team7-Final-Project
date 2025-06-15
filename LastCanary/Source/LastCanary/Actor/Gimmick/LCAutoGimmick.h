#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCTransformGimmick.h"
#include "LCAutoGimmick.generated.h"

UENUM(BlueprintType)
enum class EGimmickLoopType : uint8
{
	None UMETA(DisplayName = "None"),
	LoopForward UMETA(DisplayName = "Forward Only"),
	PingPong UMETA(DisplayName = "Back and Forth")
};

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCAutoGimmick : public ALCTransformGimmick
{
	GENERATED_BODY()

public:
	ALCAutoGimmick();

protected:
	virtual void BeginPlay() override;

public:
	/** BeginPlay 시 자동 루프 시작 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto")
	bool bStartAtBeginPlay;

	/** 루프 실행 여부 */
	UPROPERTY(BlueprintReadOnly, Category = "Auto")
	bool bLoopingEnabled;

	/** 루프 간 대기 시간 (0이면 즉시 다음 루프) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto", meta = (ClampMin = "0.0"))
	float LoopInterval;

	/** 루프 타입 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto")
	EGimmickLoopType LoopType;

	/** 정지된 후 루프를 몇 초 후에 재시작할지 (0이면 완전 멈춤) */	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Auto")
	float LoopRestartDelay;

	// Auto 루프에서 PingPong 속도 분기용 UI 세팅 변수만 추가
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Loop")
	float ForwardMoveDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Loop")
	float BackwardMoveDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Loop")
	float ForwardRotationDuration;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Loop")
	float BackwardRotationDuration;

	/** 복귀 이동 완료 시 루프 재시작 처리 */
	void CompleteReturn();

	/** 복귀 회전 완료 시 루프 재시작 처리 */
	void CompleteRotationReturn();

protected:
	FTimerHandle LoopTimerHandle;

	UFUNCTION()
	void HandleLoop();

	void ScheduleNextLoop();

public:
	UFUNCTION(BlueprintCallable, Category = "Auto")
	void StartLoop();

	UFUNCTION(BlueprintCallable, Category = "Auto")
	void StopLoop();

protected:
	virtual void ActivateGimmick_Implementation() override;
	virtual void DeactivateGimmick_Implementation() override;
	virtual void ReturnToInitialState_Implementation() override;

	virtual void CompleteMovement() override;
	virtual void CompleteRotation() override;
};