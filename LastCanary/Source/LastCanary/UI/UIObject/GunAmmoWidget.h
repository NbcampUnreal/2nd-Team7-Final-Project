#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "GameplayTagContainer.h"
#include "DataTable/GunDataRow.h"
#include "GunAmmoWidget.generated.h"

/**
 * 
 */
class UTextBlock;
class AGunBase;
class UProgressBar;
class UBorder;

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
    
    UPROPERTY(meta = (BindWidget))
    UBorder* SingleFireBorder;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* SingleFireText;

    UPROPERTY(meta = (BindWidget))
    UBorder* AutoFireBorder;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AutoFireText;

    UFUNCTION(BlueprintCallable)
    void UpdateFireModeDisplay(EFireMode CurrentMode, const TArray<EFireMode>& AvailableModes);

    UFUNCTION(BlueprintCallable)
    void UpdateAmmoDisplay(int32 CurrentAmmo, int32 TotalAmmo, int32 MagazineCapacity);

    UFUNCTION(BlueprintCallable)
    void ShowAmmoUI(int32 CurrentAmmo, int32 TotalAmmo, int32 MagazineCapacity, EFireMode CurrentMode, const TArray<EFireMode>& AvailableModes);

    UFUNCTION(BlueprintCallable)
    void UpdateAmmoUI();

    UFUNCTION(BlueprintCallable)
    void HideAmmoUI();

protected:
    // 색상 설정
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Mode Colors")
    FLinearColor ActiveModeColor = FLinearColor(0.0f, 1.0f, 0.0f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Mode Colors")
    FLinearColor AvailableModeColor = FLinearColor(0.5f, 0.5f, 0.5f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fire Mode Colors")
    FLinearColor UnavailableModeColor = FLinearColor(0.3f, 0.0f, 0.0f, 1.0f);

    UPROPERTY()
    FTimerHandle RetryTimerHandle;

    int32 RetryCount = 0;
    static const int32 MaxRetryCount = 3;

private:
    void SetBorderColor(UBorder* Border, const FLinearColor& Color);
    void InitializeFireModeUI();

public:
    virtual void NativeConstruct() override;
};
