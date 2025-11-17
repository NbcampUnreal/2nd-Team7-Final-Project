//#pragma once
//
//#include "CoreMinimal.h"
//#include "Actor/Gimmick/LCBaseGimmick.h"
//#include "LCWeightPlate.generated.h"
//
///**
// * 
// */
//class UBoxComponent;
//UCLASS()
//class LASTCANARY_API ALCWeightPlate : public ALCBaseGimmick
//{
//	GENERATED_BODY()
//	
//public:
//	ALCWeightPlate();
//
//protected:
//	virtual void BeginPlay() override;
//
//	/** 기믹이 다시 되돌릴 수 있는지 여부 */
//	UPROPERTY(EditAnywhere, Category = "Gimmick")
//	bool bRevertible = true;
//
//	/** 트리거 영역 */
//	UPROPERTY(VisibleAnywhere, Category = "Components")
//	UBoxComponent* TriggerVolume;
//
//	/** 현재 올라간 총 무게 */
//	UPROPERTY(VisibleInstanceOnly, Category = "Weight")
//	float CurrentWeight;
//
//	/** 발동 조건 무게 */
//	UPROPERTY(EditAnywhere, Category = "Weight")
//	float RequiredWeight = 100.f;
//
//	/** 최대 무게 (이 무게일 때 최대로 내려감) */
//	UPROPERTY(EditAnywhere, Category = "Weight Plate")
//	float MaxWeight = 100.f; 
//
//	/**최대 얼마나 내려갈지 (Z 축 방향) */
//	UPROPERTY(EditAnywhere, Category = "Weight Plate")
//	float MaxOffsetZ = -30.f;  
//
//	FVector InitialLocation;
//
//	FTimerHandle SmoothMoveHandle;
//
//	UPROPERTY(EditAnywhere, Category = "Weight Plate")
//	float MoveDuration = 0.5f; // 부드럽게 이동할 총 시간
//
//	UPROPERTY(EditAnywhere, Category = "Weight Plate")
//	float MoveStepInterval = 0.02f; // 0.02초마다 이동 (부드러움 정도)
//
//	FVector TargetLocation;
//	FVector StartLocation;
//	float MoveElapsed = 0.f;
//
//	/** 현재 발판 위에 올라간 액터 목록 */
//	UPROPERTY()
//	TArray<AActor*> OverlappingActors;
//
//	/** 오버랩 진입 시 */
//	UFUNCTION()
//	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
//		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
//		bool bFromSweep, const FHitResult& SweepResult);
//
//	/** 오버랩 종료 시 */
//	UFUNCTION()
//	void OnEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
//		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
//
//	/** 액터로부터 무게 추출 */
//	float GetActorWeight(AActor* Actor) const;
//
//	/** 무게 확인 및 퍼즐 처리 */
//	void CheckWeight();
//
//	void UpdateSmoothMove();
//
//	virtual void DeactivateGimmick_Implementation() override;
//};
//
