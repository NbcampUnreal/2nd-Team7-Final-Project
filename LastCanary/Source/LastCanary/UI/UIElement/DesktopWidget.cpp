#include "UI/UIElement/DesktopWidget.h"
#include "Components/Button.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UDesktopWidget::NativeConstruct()
{
    Super::NativeConstruct();

    if (ShopIconButton)
    {
        ShopIconButton->OnClicked.AddUniqueDynamic(this, &UDesktopWidget::OnShopIconClicked);
    }
    if (CloseDesktopButton)
    {
        CloseDesktopButton->OnClicked.AddUniqueDynamic(this, &UDesktopWidget::OnCloseDesktopClicked);
    }

}

void UDesktopWidget::OnShopIconClicked()
{
    if (ALCRoomPlayerController* RoomPC = GetRoomPC())
    {
        RoomPC->Server_ShowShopWidget(); // 서버에 요청
    }
}

void UDesktopWidget::OnCloseDesktopClicked()
{
    if (ULCUIManager* UIManager = ResolveUIManager())
    {
        UIManager->HideDesktop();
    }
}

ALCRoomPlayerController* UDesktopWidget::GetRoomPC() const
{
    return Cast<ALCRoomPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}