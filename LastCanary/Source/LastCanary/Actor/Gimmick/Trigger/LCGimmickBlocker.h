#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCGimmickBlocker.generated.h"

class UBoxComponent;

UCLASS(Blueprintable, BlueprintType)
class LASTCANARY_API ALCGimmickBlocker : public AActor
{
	GENERATED_BODY()
	
public:
	ALCGimmickBlocker();

protected:
	virtual void BeginPlay() override;

	/** 감지용 박스 컴포넌트 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Blocker")
	UBoxComponent* BlockVolume;

	/** 오버랩 이벤트 핸들러 */
	UFUNCTION()
	void OnBlockVolumeBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
};