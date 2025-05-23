#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCTimeManager.generated.h"

UENUM(BlueprintType)
enum class EDayPhase : uint8
{
	Day,
	Night
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTimePhaseChanged, EDayPhase, NewPhase);

UCLASS()
class LASTCANARY_API ALCTimeManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ALCTimeManager();

protected:
	virtual void BeginPlay() override;

	/** 서버에서 타이머로 주기적 시간 업데이트 */
	void UpdateTime();
	void CheckTimePhaseTransition();
	void SetPhase(EDayPhase NewPhase);

	UFUNCTION()
	void OnRep_ElapsedTime();
	UFUNCTION()
	void OnRep_ElapsedDay();
	UFUNCTION()
	void OnRep_DayPhase();

public:
	/** 디버그 로그 출력 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugLog = false;

	/** 현재 경과 시간 */
	UPROPERTY(ReplicatedUsing = OnRep_ElapsedTime, BlueprintReadOnly, Category = "Time")
	float ElapsedTime;
	
	/** 현재 경과 일수 */
	UPROPERTY(ReplicatedUsing = OnRep_ElapsedDay, BlueprintReadOnly, Category = "Time")
	int32 ElapsedDay;

	/** 현재 낮/밤 상태 */
	UPROPERTY(ReplicatedUsing = OnRep_DayPhase, BlueprintReadOnly, Category = "Time")
	EDayPhase CurrentPhase;

	/** 낮 시간 길이 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float DayDuration = 300.0f;

	/** 밤 시간 길이 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float NightDuration = 300.0f;

	/** 시간 배속 (1초 * TimeScale = 가상 초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float TimeScale = 60.0f;

	/** 시간 갱신 주기 (초) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time")
	float UpdateInterval = 1.0f;

	UPROPERTY(BlueprintAssignable, Category = "Time")
	FOnTimePhaseChanged OnTimePhaseChanged;

private:
	UPROPERTY()
	EDayPhase CachedPhase;

	FTimerHandle TimerHandle_UpdateTime;
};
