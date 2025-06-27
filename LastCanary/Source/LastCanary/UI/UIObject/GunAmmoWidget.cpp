#include "UI/UIObject/GunAmmoWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
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
    UpdateGunTypeImage();
}

void UGunAmmoWidget::UpdateGunTypeImage()
{
    if (!GunTypeImage || !CurrentGun)
    {
        return;
    }

    FGameplayTag GunItemType = CurrentGun->GetItemType();

    for (const FGunTypeImageData& ImageData : GunTypeImages)
    {
        if (ImageData.GunTypeTag.MatchesTagExact(GunItemType))
        {
            if (UTexture2D* LoadedTexture = ImageData.GunTypeImage.LoadSynchronous())
            {
                GunTypeImage->SetBrushFromTexture(LoadedTexture);
                GunTypeImage->SetVisibility(ESlateVisibility::Visible);
                return;
            }
        }
    }

    GunTypeImage->SetVisibility(ESlateVisibility::Hidden);
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

    CurrentAmmoText->SetText(FText::AsNumber(CurrentGun->GetCurrentAmmo()));
    MaxAmmoText->SetText(FText::AsNumber(CurrentGun->GetMaxAmmo()));

    LOG_Item_WARNING(TEXT("[UpdateAmmoDisplay] 탄약 UI 업데이트: %d/%d"), CurrentGun->GetCurrentAmmo(), CurrentGun->GetMaxAmmo());
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
