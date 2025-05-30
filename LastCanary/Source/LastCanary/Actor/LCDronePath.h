#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LCDronePath.generated.h"

class USplineComponent;
UCLASS()
class LASTCANARY_API ALCDronePath : public AActor
{
	GENERATED_BODY()
	
public:
	ALCDronePath();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spline")
	USplineComponent* SplinePath;

	UFUNCTION(BlueprintCallable, Category = "Spline")
	USplineComponent* GetSpline() const { return SplinePath; }
};