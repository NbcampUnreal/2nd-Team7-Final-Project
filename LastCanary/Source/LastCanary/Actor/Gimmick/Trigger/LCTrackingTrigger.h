#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCTrackingTrigger.generated.h"

class ALCTrackingManager;
class UBoxComponent;

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCTrackingTrigger : public ALCBaseGimmick
{
	GENERATED_BODY()

public:
	ALCTrackingTrigger();

protected:
	virtual void BeginPlay() override;

	/** 연결된 TrackingManager */
	UPROPERTY(EditAnywhere, Category = "Tracking")
	ALCTrackingManager* TrackingManager;

	/** 감지 시작 */
	virtual void OnTriggerEnter(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;

	/** 감지 종료 */
	virtual void OnTriggerExit(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;
};