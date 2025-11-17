#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "WeightTestActor.generated.h"

class UStaticMeshComponent;
UCLASS()
class LASTCANARY_API AWeightTestActor : public AActor
{
	GENERATED_BODY()
	
public:
	AWeightTestActor();

protected:
	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* Mesh;
	
	/** 실제 물리 질량 (kg) - 에디터에서 수정 가능 */
	UPROPERTY(EditAnywhere, Category = "Weight", meta = (ClampMin = "0.1"))
	float CustomMass = 50.0f;

	virtual void BeginPlay() override;

public:
	/** 질량을 설정해주는 함수 */
	void UpdateMass();
};