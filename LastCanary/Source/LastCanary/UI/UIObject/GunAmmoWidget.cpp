#include "UI/UIObject/GunAmmoWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Item/EquipmentItem/GunBase.h"
#include "LastCanary.h"

void UGunAmmoWidget::SetGunReference(AGunBase* Gun)
{
    if (CurrentGun)
    {
        CurrentGun->OnAmmoChanged.RemoveDynamic(this, &UGunAmmoWidget::UpdateAmmoDisplay);
    }
    CurrentGun = Gun;

    if (CurrentGun)
    {
        CurrentGun->OnAmmoChanged.AddUniqueDynamic(this, &UGunAmmoWidget::UpdateAmmoDisplay);
    }
    UpdateAmmoDisplay();
}

void UGunAmmoWidget::UpdateAmmoDisplay()
{
    if (!CurrentGun || !CurrentAmmoText || !MaxAmmoText)
    {
        if (!CurrentGun)
        {
            SetVisibility(ESlateVisibility::Hidden);
        }
        return;
    }
    
	const int32 CurrentAmmo = CurrentGun->GetCurrentAmmo();
	const int32 MaxAmmo = CurrentGun->GetMaxAmmo();

    CurrentAmmoText->SetText(FText::AsNumber(CurrentAmmo));
    MaxAmmoText->SetText(FText::AsNumber(MaxAmmo));
    
    if (AmmoProgressBar && MaxAmmo > 0)
    {
        const float AmmoRatio = static_cast<float>(CurrentAmmo) / static_cast<float>(MaxAmmo);
        AmmoProgressBar->SetPercent(AmmoRatio);
    }

    LOG_Item_WARNING(TEXT("[UpdateAmmoDisplay] 탄약 UI 업데이트: %d/%d"), CurrentAmmo, MaxAmmo);
}

void UGunAmmoWidget::ShowAmmoUI(AGunBase* Gun)
{
    SetGunReference(Gun);
    SetVisibility(ESlateVisibility::Visible);
}

void UGunAmmoWidget::HideAmmoUI()
{
    SetGunReference(nullptr);
    SetVisibility(ESlateVisibility::Hidden);
}
