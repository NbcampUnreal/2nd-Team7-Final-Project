#pragma once

#include "CoreMinimal.h"
#include "Art/Ruins/RuinsGimmickBase.h"
#include "RuinsRotatingStatue.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ARuinsRotatingStatue : public ARuinsGimmickBase
{
	GENERATED_BODY()

public:
	ARuinsRotatingStatue();

protected:
	virtual void Tick(float DeltaSeconds) override;

	// 기믹 활성화 시 회전 처리
	virtual void ActivateGimmick_Implementation() override;

	// 회전 스텝 단위 (예: 45도씩 회전)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Rotation")
	float RotationStep;

	// 현재 회전 인덱스 (0 → 45도 → 90도 ...)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statue|Rotation")
	int32 RotationIndex;

	// 회전 속도 (Yaw 보간 속도, deg/sec)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Statue|Rotation")
	float RotateSpeed;

	// 현재 회전 진행 여부
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statue|Rotation")
	bool bIsRotating;

	// 목표 회전 각도 (Yaw)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Statue|Rotation")
	float TargetYaw;

protected:
	// 회전 시작 처리
	void StartRotation();

	// 회전 완료 처리
	void FinishRotation();
};