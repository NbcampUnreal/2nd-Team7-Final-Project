#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCSimultaneousPlate.generated.h"

/**
 *
 */
class UBoxComponent;
UCLASS()
class LASTCANARY_API ALCSimultaneousPlate : public ALCBaseGimmick
{
	GENERATED_BODY()

public:
	ALCSimultaneousPlate();

protected:
	virtual void BeginPlay() override;

	/** 트리거 감지 범위 */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Gimmick")
	UBoxComponent* TriggerVolume;

	/** 필요한 최소 인원 수 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Gimmick")
	int32 RequiredCount = 2;

	/** 현재 올라와 있는 액터들 */
	UPROPERTY(VisibleInstanceOnly, Category = "Gimmick")
	TSet<AActor*> OverlappingActors;

	/** 오버랩 이벤트 처리 */
	UFUNCTION()
	void OnBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	void OnEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

	void CheckActivation();

public:
	/** 조건 검사 */
	virtual bool CanActivate_Implementation() override;
};