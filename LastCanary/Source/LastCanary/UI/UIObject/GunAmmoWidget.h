#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "GameplayTagContainer.h"
#include "GunAmmoWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class AGunBase;
class UProgressBar;

UCLASS()
class LASTCANARY_API UGunAmmoWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(meta = (BindWidget))
    UTextBlock* CurrentAmmoText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* MaxAmmoText;

    UPROPERTY(meta = (BindWidget))
    UProgressBar* AmmoProgressBar;

    UPROPERTY()
    AGunBase* CurrentGun;

    UFUNCTION(BlueprintCallable)
    void UpdateAmmoDisplay();

    UFUNCTION(BlueprintCallable)
    void ShowAmmoUI(AGunBase* Gun);

    UFUNCTION(BlueprintCallable)
    void HideAmmoUI();

    void SetGunReference(AGunBase* Gun);
};
