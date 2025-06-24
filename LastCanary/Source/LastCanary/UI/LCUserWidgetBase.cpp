#include "LastCanary/UI/LCUserWidgetBase.h"
#include "LastCanary/Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/PlayerController/LCPlayerInputController.h"

#include "LastCanary.h"

ULCUIManager* ULCUserWidgetBase::ResolveUIManager() const
{
	if (const UGameInstance* GameInstance = GetGameInstance())
	{
		if (const ULCGameInstanceSubsystem* Subsystem = GameInstance->GetSubsystem<ULCGameInstanceSubsystem>())
		{
			return Subsystem->GetUIManager();
		}
	}
	LOG_Frame_WARNING(TEXT("ResolveUIManager failed : GameInstance or UI Subsystem is null"));
	return nullptr;
}

FReply ULCUserWidgetBase::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        TSharedPtr<SWidget> CachedSlateWidget = GetCachedWidget();
        if (CachedSlateWidget.IsValid())
        {
            if (APlayerController* PC = GetOwningPlayer())
            {
                if (ALCPlayerInputController* MyPC = Cast<ALCPlayerInputController>(PC))
                {
                    UE_LOG(LogTemp, Warning, TEXT("UI에서 좌클릭 감지됨"));
                    MyPC->OnUIClicked();
                }
            }
            return FReply::Handled()
                .CaptureMouse(CachedSlateWidget.ToSharedRef())
                .SetUserFocus(CachedSlateWidget.ToSharedRef(), EFocusCause::Mouse);
        }

        return FReply::Handled(); // Slate 핸들 없을 경우 그래도 처리
    }
    return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply ULCUserWidgetBase::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
    {
        if (APlayerController* PC = GetOwningPlayer())
        {
            if (ALCPlayerInputController* MyPC = Cast<ALCPlayerInputController>(PC))
            {
                UE_LOG(LogTemp, Warning, TEXT("UI에서 좌클릭 떼는 거 감지됨"));
                MyPC->OnUIReleased();
            }
        }
        return FReply::Handled().ReleaseMouseCapture();
    }
    return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}
