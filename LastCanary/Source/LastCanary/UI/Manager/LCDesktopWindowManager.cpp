#include "UI/Manager/LCDesktopWindowManager.h"
#include "UI/UIElement/DesktopWidget.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "LastCanary.h"

void ULCDesktopWindowManager::Init(UCanvasPanel* InRootCanvas)
{
	RootCanvas = InRootCanvas;
}

void ULCDesktopWindowManager::OpenWindow(UUserWidget* Widget)
{
	if (Widget == nullptr || DesktopWidget == nullptr)
	{
		return;
	}

	if (UCanvasPanel* Container = DesktopWidget->GetWindowContainer())
	{
		if (Container->GetAllChildren().Contains(Widget) == false)
		{
			DesktopWidget->AddWindow(Widget);
		}
	}

	if (UDesktopWindowBaseWidget* Window = Cast<UDesktopWindowBaseWidget>(Widget))
	{
		if (Window->IsMaximized())
		{
			const FVector2D CanvasSize = DesktopWidget->GetCachedGeometry().GetLocalSize();
			Window->AnimateWindowTransform(true, CanvasSize);

			if (UHorizontalBoxSlot* TitleTextContainerSlot = Cast<UHorizontalBoxSlot>(Window->GetTitleTextContainer()->Slot))
			{
				TitleTextContainerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
			}
		}
	}

	if (OpenedWindows.Contains(Widget) == false)
	{
		OpenedWindows.Add(Widget);
	}
}

void ULCDesktopWindowManager::CloseWindow(UUserWidget* Widget)
{
	if (Widget == nullptr)
	{
		return;
	}

	OpenedWindows.Remove(Widget);

	if (Widget->IsInViewport() || Widget->IsVisible())
	{
		Widget->RemoveFromParent();
	}
}

void ULCDesktopWindowManager::MinimizeWindow(UUserWidget* Widget)
{
	if (UDesktopWindowBaseWidget* Window = Cast<UDesktopWindowBaseWidget>(Widget))
	{
		Window->SetMinimized(true);
		Window->PlayMinimizeAnimation();
	}
}

void ULCDesktopWindowManager::ToggleMaximizeRestore(UUserWidget* Widget)
{
	if (Widget == nullptr || DesktopWidget == nullptr)
	{
		return;
	}

	if (UDesktopWindowBaseWidget* Window = Cast<UDesktopWindowBaseWidget>(Widget))
	{
		if (UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Window->Slot))
		{
			if (Window->IsMaximized())
			{
				// 복원
				Window->AnimateWindowTransform(false, FVector2D::ZeroVector); 

				if (UHorizontalBoxSlot* TitleTextContainerSlot = Cast<UHorizontalBoxSlot>(Window->GetTitleTextContainer()->Slot))
				{
					TitleTextContainerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Automatic));
				}
			}
			else
			{
				// 최대화 전 위치/크기 저장
				if (UCanvasPanelSlot* CurrentSlot = Cast<UCanvasPanelSlot>(Window->Slot))
				{
					Window->SetOriginalAnchors(CurrentSlot->GetAnchors());
					Window->SetOriginalAlignment(CurrentSlot->GetAlignment());
					Window->SetOriginalPosition(CurrentSlot->GetPosition());
					Window->SetOriginalSize(CurrentSlot->GetSize());
				}

				const FVector2D CanvasSize = DesktopWidget->GetCachedGeometry().GetLocalSize();
				Window->AnimateWindowTransform(true, CanvasSize);

				if (UHorizontalBoxSlot* TitleTextContainerSlot = Cast<UHorizontalBoxSlot>(Window->GetTitleTextContainer()->Slot))
				{
					TitleTextContainerSlot->SetSize(FSlateChildSize(ESlateSizeRule::Fill));
				}
			}
		}
	}
}

int ULCDesktopWindowManager::GetNextZOrder()
{
	return CurrentZOrder++;
}

bool ULCDesktopWindowManager::IsTopMost(UUserWidget* Widget) const
{
	if (Widget == nullptr)
	{
		return false;
	}
	if (OpenedWindows.Num() == 0)
	{
		return false;
	}

	return OpenedWindows.Last() == Widget;
}
