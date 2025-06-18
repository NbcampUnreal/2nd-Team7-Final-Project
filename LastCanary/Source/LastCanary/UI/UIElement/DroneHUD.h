#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DroneHUD.generated.h"

/**
 * 
 */
class UTextBlock;
class UProgressBar;
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

public:
	void UpdateDistanceDisplay(float Distance, float MaxDistance);

};
