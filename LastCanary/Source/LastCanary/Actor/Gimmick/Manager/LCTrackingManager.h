#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCTrackingManager.generated.h"

class ALCTrackingGimmick;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCTrackingManager : public AActor 
{
	GENERATED_BODY()

public:
	ALCTrackingManager();

	/** 감지된 플레이어 등록 */
	void RegisterPlayer(AActor* Player);

	/** 감지 해제된 플레이어 제거 */
	void UnregisterPlayer(AActor* Player);

	/** 감시탑 등록 (트리거에서 자동 등록 예정) */
	void RegisterGimmick(ALCTrackingGimmick* Gimmick);

protected:
	virtual void BeginPlay() override;

	/** 현재 감지된 플레이어 목록 */
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> DetectedPlayers;

	/** 연결된 감시탑 목록 */
	UPROPERTY(EditInstanceOnly, BlueprintReadWrite, Category = "Tracking")
	TArray<ALCTrackingGimmick*> TrackingTowers;

	/** 현재 타겟 */
	UPROPERTY()
	AActor* CurrentTarget;

	/** 추적 루프 활성 상태 */
	bool bIsTrackingActive;

	/** 타겟 지정 전 대기 시간 */
	UPROPERTY(EditAnywhere, Category = "Tracking|Timing", meta = (ClampMin = 0.1))
	float DetectionDelay;

	/** 타겟을 추적하는 전체 시간 */
	UPROPERTY(EditAnywhere, Category = "Tracking|Timing", meta = (ClampMin = 0.1))
	float TrackingDuration;

	/** 추적 중 나이아가라 발사까지의 지연 시간 */
	UPROPERTY(EditAnywhere, Category = "Tracking|Timing", meta = (ClampMin = 0.1))
	float FireOffset;

	/** 쿨타임 시간 (다음 루프까지 대기) */
	UPROPERTY(EditAnywhere, Category = "Tracking|Timing", meta = (ClampMin = 0.1))
	float CooldownDelay;

	/** 각종 타이머 */
	FTimerHandle TrackingLoopHandle;
	FTimerHandle FireTimerHandle;
	FTimerHandle TrackingStopTimerHandle;

	/** 타겟 지정 시작 */
	void ChooseRandomTarget();

	/** 모든 감시탑에 타겟 전파 */
	void UpdateAllTowers(AActor* Target);

	/** 나이아가라 발사 */
	void FireAllTowers();

	/** 추적 루프 정지 */
	void StopTrackingLoop();

	/** 추적 루프 시작 */
	void StartTrackingLoop();

	FString GetPlayerNameFromActor(AActor* Actor) const;

};