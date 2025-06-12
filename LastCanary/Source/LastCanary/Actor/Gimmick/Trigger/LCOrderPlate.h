#pragma once

#include "CoreMinimal.h"
#include "Actor/Gimmick/LCBaseGimmick.h"
#include "LCOrderPlate.generated.h"

/**
 *
 */
class UBoxComponent;
class ALCOrderSwitch;
class UMaterialInstanceDynamic;
UCLASS()
class LASTCANARY_API ALCOrderPlate : public ALCBaseGimmick
{
	GENERATED_BODY()

public:
	ALCOrderPlate();

	UFUNCTION(BlueprintCallable)
	void ApplyMaterial(bool bIsSelected);

	UPROPERTY(EditAnywhere)
	int32 PlateIndex;

protected:
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UBoxComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere, Category = "Components")
	UStaticMeshComponent* PlateMesh;

	UPROPERTY(EditAnywhere, Category = "Gimmick")
	ALCOrderSwitch* TargetSwitch;

	UFUNCTION()
	void OnBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor,
		UPrimitiveComponent* OtherComp, int32 OtherBodyIndex,
		bool bFromSweep, const FHitResult& SweepResult);

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* DefaultMaterial;

	UPROPERTY(EditAnywhere, Category = "Material")
	UMaterialInterface* SelectedMaterial;
};