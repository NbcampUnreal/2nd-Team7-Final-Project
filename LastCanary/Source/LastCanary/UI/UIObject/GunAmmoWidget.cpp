#include "UI/UIObject/GunAmmoWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Item/EquipmentItem/GunBase.h"
#include "LastCanary.h"

void UGunAmmoWidget::UpdateAmmoDisplay(int32 CurrentAmmo, int32 MaxAmmo)
{
    if (!CurrentAmmoText || !MaxAmmoText)
    {
        return;
    }

    CurrentAmmoText->SetText(FText::AsNumber(CurrentAmmo));
    MaxAmmoText->SetText(FText::AsNumber(MaxAmmo));

    if (AmmoProgressBar && MaxAmmo > 0)
    {
        const float AmmoRatio = static_cast<float>(CurrentAmmo) / static_cast<float>(MaxAmmo);
        AmmoProgressBar->SetPercent(AmmoRatio);
    }

    LOG_Item_WARNING(TEXT("[UpdateAmmoDisplay] 탄약 UI 업데이트: %d/%d"), CurrentAmmo, MaxAmmo);
}

void UGunAmmoWidget::ShowAmmoUI(int32 CurrentAmmo, int32 MaxAmmo)
{
    UpdateAmmoDisplay(CurrentAmmo, MaxAmmo);
    SetVisibility(ESlateVisibility::Visible);
}

void UGunAmmoWidget::HideAmmoUI()
{
    SetVisibility(ESlateVisibility::Hidden);
}
