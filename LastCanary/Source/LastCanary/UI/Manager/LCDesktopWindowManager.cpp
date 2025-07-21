#include "UI/Manager/LCDesktopWindowManager.h"
#include "UI/UIElement/DesktopWidget.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "UI/Manager/LCUIManager.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/UserWidget.h"
#include "UObject/ConstructorHelpers.h"
#include "LastCanary.h"

void ULCDesktopWindowManager::Init(UCanvasPanel* InRootCanvas)
{
	RootCanvas = InRootCanvas;
}

void ULCDesktopWindowManager::OpenWindow(UUserWidget* Widget)
{
	if (!Widget)
	{
		LOG_Frame_WARNING(TEXT("ULCDesktopWindowManager::OpenWindow - Widget null."));
		return;
	}
	if (!DesktopWidget)
	{
		LOG_Frame_WARNING(TEXT("ULCDesktopWindowManager::OpenWindow - DesktopWidget null."));
		return;
	}

	DesktopWidget->AddWindow(Widget);
	OpenedWindows.Add(Widget);
}

void ULCDesktopWindowManager::CloseWindow(UUserWidget* Widget)
{
	if (UDesktopWindowBaseWidget* Window = Cast<UDesktopWindowBaseWidget>(Widget))
	{
		Window->CloseWindow();
	}
	OpenedWindows.Remove(Widget);
}

void ULCDesktopWindowManager::ToggleMaximizeRestore(UUserWidget* Widget)
{
	if (UDesktopWindowBaseWidget* Window = Cast<UDesktopWindowBaseWidget>(Widget))
	{
		Window->ToggleMaximizeRestore();
	}
}
