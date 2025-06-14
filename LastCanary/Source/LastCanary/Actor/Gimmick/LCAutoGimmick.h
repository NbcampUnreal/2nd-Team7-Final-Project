#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCTransformGimmick.h"
#include "LCAutoGimmick.generated.h"

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

	/** 반복 실행 주기 시작 (서버 전용) */
	void StartLoop();

	/** 한 번 이동 수행 */
	void AutoMove();

	/** 한 번 복귀 수행 */
	void AutoReturn();

	/** 자동 반복 여부 (외부에서 비활성화 가능) */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Auto")
	bool bLoopingEnabled;

	/** 외부에서 반복 중단 */
	UFUNCTION(BlueprintCallable, Category = "Gimmick|Auto")
	void StopLoop();

	/** 외부에서 반복 재시작 */
	UFUNCTION(BlueprintCallable, Category = "Gimmick|Auto")
	void ResumeLoop();

	/** 이동 완료 후 복귀를 위한 딜레이 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Auto")
	float MovePauseTime;

	/** 복귀 완료 후 재시작까지 대기 시간 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick|Auto")
	float ReturnPauseTime;

	FTimerHandle AutoMoveTimerHandle;
	FTimerHandle AutoReturnTimerHandle;
	
};
