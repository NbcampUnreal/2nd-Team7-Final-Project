#pragma once

#include "CoreMinimal.h"
#include "Character/Component/CharacterBaseComponent.h"
#include "Components/TimelineComponent.h"
#include "CameraRecoilComponent.generated.h"

UCLASS()
class LASTCANARY_API UCameraRecoilComponent : public UCharacterBaseComponent
{
	GENERATED_BODY()
	

protected:
	UCameraRecoilComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;


	void ApplyRecoil(float RecoilAmount);

	// Timeline 객체
	FTimeline RecoilTimeline;

	// Curve Asset (float 타입)
	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* RecoilCurve;

	UFUNCTION()
	void StartRecoil(float RecoilAmount);

	UFUNCTION()
	void OnRecoilFinished();

	UFUNCTION()
	void HandleRecoilFloat(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil|Multiplier")
	float DefaultRecoilMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil|Multiplier")
	float ADSRecoilMultiplier = 0.4f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recoil|Multiplier")
	float GunRecoilMultiplier = 1.0f;
};
