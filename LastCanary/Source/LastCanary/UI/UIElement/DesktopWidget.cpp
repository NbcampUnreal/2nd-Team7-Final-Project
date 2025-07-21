#include "UI/UIElement/DesktopWidget.h"
#include "UI/UIObject/TaskbarWidget.h"
#include "Components/Button.h"
#include "Components/Overlay.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCDesktopWindowManager.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Kismet/GameplayStatics.h"

void UDesktopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (WindowManager == nullptr)
	{
		WindowManager = NewObject<ULCDesktopWindowManager>(this);
		WindowManager->Init(this->GetRootCanvas()); 
	}
	if (CloseDesktopButton)
	{
		CloseDesktopButton->OnClicked.AddUniqueDynamic(this, &UDesktopWidget::OnCloseDesktopClicked);
	}
	if (TaskbarWidget && TaskbarAppButtonClass)
	{
		TaskbarWidget->SetTaskbarAppButtonClass(TaskbarAppButtonClass);
	}
	if (ShopIconButton)
	{
		ShopIconButton->OnClicked.AddUniqueDynamic(this, &UDesktopWidget::OnShopIconSingleClicked);
	}

	ClickCount = 0;
}

FReply UDesktopWidget::NativeOnMouseButtonDoubleClick(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		if (ShopIconButton && ShopIconButton->IsHovered())
		{
			LOG_Frame_WARNING(TEXT("ShopIcon Double Clicked! Opening Shop App"));
			HandleShopAppLaunch();
			return FReply::Handled();
		}
	}

	return Super::NativeOnMouseButtonDoubleClick(InGeometry, InMouseEvent);
}

void UDesktopWidget::OnShopIconSingleClicked()
{
	ClickCount++;

	// 첫 클릭 시 테두리 강조
	if (ClickCount == 1)
	{
		LOG_Frame_WARNING(TEXT("ShopIcon First Click - Highlighting"));

		const FLinearColor HighlightColor = FLinearColor::Yellow;

		FButtonStyle ButtonStyle = ShopIconButton->WidgetStyle;
		ButtonStyle.Normal.TintColor = FSlateColor(HighlightColor);
		ButtonStyle.Hovered.TintColor = FSlateColor(HighlightColor);
		ButtonStyle.Pressed.TintColor = FSlateColor(HighlightColor);

		ShopIconButton->SetStyle(ButtonStyle);

		// 일정 시간 후 클릭 수 초기화
		GetWorld()->GetTimerManager().SetTimer(DoubleClickTimerHandle, this, &UDesktopWidget::ResetClickCount, 0.3f, false);
	}
	else if (ClickCount == 2)
	{
		LOG_Frame_WARNING(TEXT("ShopIcon Double Click - Opening Shop"));
		GetWorld()->GetTimerManager().ClearTimer(DoubleClickTimerHandle); // 초기화 타이머 제거
		ClickCount = 0;

		HandleShopAppLaunch();
	}
}

void UDesktopWidget::ResetClickCount()
{
	ClickCount = 0;
}

void UDesktopWidget::HandleShopAppLaunch()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(PC))
		{
			RoomPC->Server_ShowShopWidget();
		}
		else
		{
			LOG_Frame_WARNING(TEXT("HandleShopAppLaunch failed: Not a LCRoomPlayerController."));
		}
	}
	else
	{
		LOG_Frame_WARNING(TEXT("HandleShopAppLaunch failed: No OwningPlayer."));
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

void UDesktopWidget::AddWindow(UUserWidget* NewWindow)
{
	if (NewWindow == nullptr)
	{
		LOG_Frame_WARNING(TEXT("AddWindow: NewWindow is null"));
		return;
	}

	if (WindowContainer == nullptr) // = [보더] 또는 그 아래 적절한 Panel
	{
		LOG_Frame_WARNING(TEXT("AddWindow: WindowContainer is null"));
		return;
	}

	WindowContainer->AddChild(NewWindow);
}
