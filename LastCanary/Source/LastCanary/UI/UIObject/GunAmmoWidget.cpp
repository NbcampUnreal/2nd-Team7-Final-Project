#include "UI/UIObject/GunAmmoWidget.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/Border.h"
#include "Inventory/ToolbarInventoryComponent.h"
#include "Item/EquipmentItem/GunBase.h"
#include "LastCanary.h"

void UGunAmmoWidget::NativeConstruct()
{
    Super::NativeConstruct();
    InitializeFireModeUI();
}

void UGunAmmoWidget::UpdateFireModeDisplay(EFireMode CurrentMode, const TArray<EFireMode>& AvailableModes)
{
    if (!SingleFireBorder || !AutoFireBorder || !SingleFireText || !AutoFireText)
    {
        LOG_Item_WARNING(TEXT("[UpdateFireModeDisplay] 발사 모드 UI 요소가 null"));
        return;
    }

    // 단발 모드 처리
    bool bSingleAvailable = AvailableModes.Contains(EFireMode::Single);
    bool bSingleActive = (CurrentMode == EFireMode::Single);

    if (bSingleActive)
    {
        SetBorderColor(SingleFireBorder, ActiveModeColor);
    }
    else if (bSingleAvailable)
    {
        SetBorderColor(SingleFireBorder, AvailableModeColor);
    }
    else
    {
        SetBorderColor(SingleFireBorder, UnavailableModeColor);
    }

    // 연발 모드 처리
    bool bAutoAvailable = AvailableModes.Contains(EFireMode::FullAuto);
    bool bAutoActive = (CurrentMode == EFireMode::FullAuto);

    if (bAutoActive)
    {
        SetBorderColor(AutoFireBorder, ActiveModeColor);
    }
    else if (bAutoAvailable)
    {
        SetBorderColor(AutoFireBorder, AvailableModeColor);
    }
    else
    {
        SetBorderColor(AutoFireBorder, UnavailableModeColor);
    }

    LOG_Item_WARNING(TEXT("[UpdateFireModeDisplay] 발사 모드 UI 업데이트 완료 - 현재: %d"), (int32)CurrentMode);
}

void UGunAmmoWidget::UpdateAmmoDisplay(int32 CurrentAmmo, int32 TotalAmmo, int32 MagazineCapacity)
{
    if (!CurrentAmmoText || !MaxAmmoText)
    {
        return;
    }

    CurrentAmmoText->SetText(FText::AsNumber(CurrentAmmo));
    MaxAmmoText->SetText(FText::AsNumber(TotalAmmo));

    if (AmmoProgressBar && MagazineCapacity > 0)
    {
        float Ratio = float(CurrentAmmo) / float(MagazineCapacity);
        AmmoProgressBar->SetPercent(FMath::Clamp(Ratio, 0.0f, 1.0f));
    }

    LOG_Item_WARNING(TEXT("[UpdateAmmoDisplay] 탄약 UI 업데이트: %d/%d"), CurrentAmmo, TotalAmmo);
}

void UGunAmmoWidget::ShowAmmoUI(int32 CurrentAmmo, int32 TotalAmmo, int32 MagazineCapacity, EFireMode CurrentMode, const TArray<EFireMode>& AvailableModes)
{
    UpdateAmmoDisplay(CurrentAmmo, TotalAmmo, MagazineCapacity);
    
    if (CurrentMode != EFireMode::None)
    {
        UpdateFireModeDisplay(CurrentMode, AvailableModes);
    }

    SetVisibility(ESlateVisibility::Visible);
}

void UGunAmmoWidget::UpdateAmmoUI()
{
    LOG_Item_WARNING(TEXT("[UpdateAmmoUI] 함수 시작"));

    APlayerController* PC = GetOwningPlayer();
    if (!PC || !PC->GetPawn())
    {
        LOG_Item_WARNING(TEXT("[UpdateAmmoUI] PlayerController를 찾을 수 없음"));
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    // 현재 장착된 총기가 있는지 확인
    UToolbarInventoryComponent* ToolbarComp = PC->GetPawn()->FindComponentByClass<UToolbarInventoryComponent>();
    if (!ToolbarComp)
    {
        LOG_Item_WARNING(TEXT("[UpdateAmmoUI] ToolbarComponent를 찾을 수 없음"));
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    // 현재 장착된 아이템 확인
    AItemBase* CurrentEquippedItem = ToolbarComp->GetCurrentEquippedItem();
    if (!CurrentEquippedItem)
    {
        LOG_Item_WARNING(TEXT("[UpdateAmmoUI] 장착된 아이템 없음"));
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    // 총기인지 확인
    AGunBase* CurrentGun = Cast<AGunBase>(CurrentEquippedItem);
    if (!CurrentGun)
    {
        LOG_Item_WARNING(TEXT("[UpdateAmmoUI] 장착된 아이템이 총기가 아님"));
        SetVisibility(ESlateVisibility::Hidden);
        return;
    }

    // 직접 총기에서 데이터 가져오기
    int32 CurrentAmmo = CurrentGun->GetCurrentAmmo();
    int32 TotalAmmo = CurrentGun->GetReserveAmmo();
    int32 MagazineCapacity = CurrentGun->GetMagazineCapacity();
    EFireMode CurrentMode = CurrentGun->GetCurrentFireMode();
    TArray<EFireMode> AvailableModes = CurrentGun->GetAvailableFireModes();
    
    LOG_Item_WARNING(TEXT("[UpdateAmmoUI] 탄약 정보: %d/%d (용량: %d)"),
        CurrentAmmo, TotalAmmo, MagazineCapacity);

    // UI 업데이트
    UpdateAmmoDisplay(CurrentAmmo, TotalAmmo, MagazineCapacity);
    UpdateFireModeDisplay(CurrentMode, AvailableModes);
    SetVisibility(ESlateVisibility::Visible);
}

void UGunAmmoWidget::HideAmmoUI()
{
    SetVisibility(ESlateVisibility::Hidden);
}

void UGunAmmoWidget::InitializeFireModeUI()
{
    if (SingleFireText)
    {
        SingleFireText->SetText(FText::FromString(TEXT("단발")));
    }

    if (AutoFireText)
    {
        AutoFireText->SetText(FText::FromString(TEXT("연발")));
    }

    // 초기 색상 설정 (비활성 상태)
    SetBorderColor(SingleFireBorder, UnavailableModeColor);
    SetBorderColor(AutoFireBorder, UnavailableModeColor);
}

void UGunAmmoWidget::SetBorderColor(UBorder* Border, const FLinearColor& Color)
{
    if (!Border)
    {
        return;
    }
    FSlateBrush NewBrush;
    NewBrush.DrawAs = ESlateBrushDrawType::RoundedBox;
    NewBrush.TintColor = FSlateColor(Color);

    // 둥글기 설정 (모든 모서리 동일)
    NewBrush.OutlineSettings.CornerRadii = FVector4(
        10.0f, 10.0f,
        10.0f, 10.0f
    );

    // 두께 설정 (선택사항)
    NewBrush.OutlineSettings.Width = 2.0f;
    NewBrush.OutlineSettings.Color = FSlateColor(Color * 0.8f); // 약간 어두운 테두리

    Border->SetBrush(NewBrush);
    // 보더의 브러시 색상 설정
    Border->SetBrushColor(Color);
}
