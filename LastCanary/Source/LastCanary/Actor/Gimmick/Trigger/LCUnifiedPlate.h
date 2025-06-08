//#pragma once
//
//#include "CoreMinimal.h"
//#include "Actor/Gimmick/LCBaseGimmick.h"
//#include "DataType/GimmickActivationType.h"
//#include "LCUnifiedPlate.generated.h"
//
///**
// * 
// */
//class UBoxComponent;
//UCLASS()
//class LASTCANARY_API ALCUnifiedPlate : public ALCBaseGimmick
//{
//	GENERATED_BODY()
//	
//public:
//	ALCUnifiedPlate();
//
//protected:
//	virtual void BeginPlay() override;
//
//	UPROPERTY(VisibleAnywhere)
//	UBoxComponent* TriggerVolume;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick")
//	int32 RequiredCount = 1;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick", meta = (EditCondition = "ActivationType == EGimmickActivationType::ActivateAfterDelay", ClampMin = "0.1"))
//	float ActivationDelay = 2.0f;
//
//	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gimmick", meta = (EditCondition = "ActivationType == EGimmickActivationType::ActivateTimed", ClampMin = "0.1"))
//	float ActiveDuration = 3.0f;
//
//	UPROPERTY()
//	TSet<AActor*> OverlappingActors;
//
//	FTimerHandle ActivationDelayHandle;
//	FTimerHandle ActiveDurationHandle;
//
//	UFUNCTION()
//	void OnBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
//		bool bFromSweep, const FHitResult& SweepResult);
//
//	UFUNCTION()
//	void OnEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
//		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
//
//	void UpdateActivationState();
//
//	void TryActivateAfterDelay();
//	void OnTimedActivationExpired();
//
//public:
//	virtual void DeactivateGimmick_Implementation() override;
//};
