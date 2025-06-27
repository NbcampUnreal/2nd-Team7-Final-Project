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
class UImage;

USTRUCT(BlueprintType)
struct FGunTypeImageData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Type")
    FGameplayTag GunTypeTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Type")
    TSoftObjectPtr<UTexture2D> GunTypeImage;

    FGunTypeImageData()
    {
        GunTypeTag = FGameplayTag::EmptyTag;
        GunTypeImage = nullptr;
    }
};

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
    UImage* GunTypeImage;

    UPROPERTY()
    AGunBase* CurrentGun;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gun Type Images")
    TArray<FGunTypeImageData> GunTypeImages;

    UFUNCTION(BlueprintCallable)
    void UpdateAmmoDisplay();

    UFUNCTION(BlueprintCallable)
    void ShowAmmoUI(AGunBase* Gun);

    UFUNCTION(BlueprintCallable)
    void HideAmmoUI();

    void SetGunReference(AGunBase* Gun);

private:
    void UpdateGunTypeImage();
};
