#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DroneHUD.generated.h"

/**
 * 
 */
class UTextBlock;
class UProgressBar;
class UInputAction;
UCLASS()
class LASTCANARY_API UDroneHUD : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* DistanceText;
	UPROPERTY(meta = (BindWidget))
	UProgressBar* DistanceProgressBar;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* InstructionText; // "Press [U] to Exit Drone"

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input")
	UInputAction* IA_ExitDrone;

public:
	void UpdateDistanceDisplay(float Distance, float MaxDistance);
	void UpdateInstructionText();

private:
	FString GetCurrentKeyNameForAction(UInputAction* InputAction) const;
};
