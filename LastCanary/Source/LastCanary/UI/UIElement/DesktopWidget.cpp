#include "UI/UIElement/DesktopWidget.h"
#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/TaskbarWidget.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"
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
			ULCUIManager* UIManager = RoomPC->GetUIManager();
			if (UIManager == nullptr)
			{
				return;
			}

			if (UShopWidget* ShopWidget = UIManager->GetCachedShopWidget()) // 이미 생성된 경우
			{
				if (ULCDesktopWindowManager* LCDesktopWindowManager = UIManager->GetDesktopWindowManager())
				{
					if (ShopWidget->IsMinimized())
					{
						// 최소화되어 있으면 복원
						ShopWidget->SetVisibility(ESlateVisibility::Visible);
						ShopWidget->SetMinimized(false);
						ShopWidget->PlayRestoreAnimation();

					}

					WindowManager->OpenWindow(ShopWidget);
				}
			}
			else
			{
				// 없는 경우 서버에 위젯 요청 (Gold 정보 포함)
				RoomPC->Server_ShowShopWidget();
			}
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
	if (NewWindow == nullptr || WindowContainer == nullptr)
	{
		LOG_Frame_WARNING(TEXT("AddWindow: NewWindow or WindowContainer is null"));
		return;
	}

	if (NewWindow->GetParent() == WindowContainer)
	{
		LOG_Frame_WARNING(TEXT("AddWindow: Already added to WindowContainer, removing first"));
		NewWindow->RemoveFromParent();
	}

	// 아직 부모에 없는 경우에만 추가
	if (NewWindow->GetParent() == nullptr)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(WindowContainer->AddChild(NewWindow)))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));      // 좌상단 기준
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));   // 좌상단 정렬
			CanvasSlot->SetPosition(FVector2D(200.f, 200.f));
			CanvasSlot->SetSize(FVector2D(625.f, 345.f));
		}
		else
		{
			LOG_Frame_WARNING(TEXT("AddWindow: Failed to cast to UCanvasPanelSlot"));
		}
	}
	else
	{
		LOG_Frame_WARNING(TEXT("AddWindow: Skipped AddChild - already has parent"));
	}

	if (UDesktopWindowBaseWidget* DesktopWindow = Cast<UDesktopWindowBaseWidget>(NewWindow))
	{
		if (WindowManager)
		{
			DesktopWindow->SetWindowManager(WindowManager);
		}
		else
		{
			LOG_Frame_WARNING(TEXT("AddWindow: WindowManager is null"));
		}
	}
}
