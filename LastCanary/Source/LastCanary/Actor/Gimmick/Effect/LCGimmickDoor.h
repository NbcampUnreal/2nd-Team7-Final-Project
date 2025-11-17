#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/GimmickEffectInterface.h"
#include "Curves/CurveFloat.h"
#include "LCGimmickDoor.generated.h"

class UStaticMeshComponent;
class UTimelineComponent;

UCLASS()
class LASTCANARY_API ALCGimmickDoor : public AActor, public IGimmickEffectInterface
{
	GENERATED_BODY()

public:
	ALCGimmickDoor();

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* DoorMesh;
	UPROPERTY(EditAnywhere, Category = "Gimmick")
	bool bSlideInstead = true;

	UPROPERTY(EditAnywhere, Category = "Gimmick", meta = (EditCondition = "bSlideInstead"))
	FVector SlideOffset;
	UPROPERTY(EditAnywhere, Category = "Gimmick", meta = (EditCondition = "!bSlideInstead"))
	FRotator OpenRotation;
	UPROPERTY(EditAnywhere, Category = "Gimmick|Animation")
	UCurveFloat* OpenCurve;

	UPROPERTY()
	UTimelineComponent* DoorTimeline;

	FVector InitialLocation;
	FVector TargetLocation;

	FRotator InitialRotation;
	FRotator TargetRotation;

	FVector ClosedLocation;
	FVector OpenedLocation;

	FRotator ClosedRotation;
	FRotator OpenedRotation;

	UFUNCTION()
	void OnDoorTimelineUpdate(float Value);

	UFUNCTION()
	void OnDoorTimelineFinished();

	/** 애니메이션 트리거 */
	void StartDoorAnimation(bool bOpen);

	bool bIsOpen = false;

public:
	virtual void TriggerEffect_Implementation() override;
	virtual void StopEffect_Implementation() override;
};
