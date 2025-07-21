#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIObject/TaskbarWidget.h"

void UDesktopWindowBaseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (MinimizeButton)
	{
		MinimizeButton->OnClicked.AddUniqueDynamic(this, &UDesktopWindowBaseWidget::OnMinimizeClicked);
	}
	if (MaximizeButton)
	{
		MaximizeButton->OnClicked.AddUniqueDynamic(this, &UDesktopWindowBaseWidget::OnMaximizeClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UDesktopWindowBaseWidget::OnCloseClicked);
	}
}

void UDesktopWindowBaseWidget::NativeDestruct()
{
	Super::NativeDestruct();

	// Taskbar에서 제거
	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		if (UTaskbarWidget* Taskbar = UIManager->GetTaskbarWidget())
		{
			Taskbar->RemoveApp(AppID);
		}
	}
}

void UDesktopWindowBaseWidget::InitDesktopWindow()
{
	// Taskbar 등록
	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		if (UTaskbarWidget* Taskbar = UIManager->GetTaskbarWidget())
		{
			Taskbar->RegisterApp(this, AppID, AppName, AppIcon);
		}
	}
}

void UDesktopWindowBaseWidget::MinimizeWindow()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

void UDesktopWindowBaseWidget::CloseWindow()
{
	RemoveFromParent();
}

void UDesktopWindowBaseWidget::ToggleMaximizeRestore()
{
	UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	if (!CanvasSlot) return;

	if (bIsMaximized)
	{
		CanvasSlot->SetPosition(OriginalPosition);
		CanvasSlot->SetSize(OriginalSize);
		bIsMaximized = false;
	}
	else
	{
		OriginalPosition = CanvasSlot->GetPosition();
		OriginalSize = CanvasSlot->GetSize();

		const FVector2D ViewportSize = UWidgetLayoutLibrary::GetViewportSize(this);
		CanvasSlot->SetPosition(FVector2D::ZeroVector);
		CanvasSlot->SetSize(ViewportSize);

		bIsMaximized = true;
	}
}

void UDesktopWindowBaseWidget::OnMinimizeClicked()
{
	MinimizeWindow();
}

void UDesktopWindowBaseWidget::OnMaximizeClicked()
{
	ToggleMaximizeRestore();
}

void UDesktopWindowBaseWidget::OnCloseClicked()
{
	CloseWindow();
}

FReply UDesktopWindowBaseWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton) && IsInTitleBar(InMouseEvent.GetScreenSpacePosition()))
	{
		StartDragging(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UDesktopWindowBaseWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging)
	{
		UpdateDrag(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UDesktopWindowBaseWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging && InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		StopDragging();
		return FReply::Handled().ReleaseMouseCapture();
	}
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UDesktopWindowBaseWidget::StartDragging(const FVector2D& InMousePos)
{
	bDragging = true;

	// 현재 위치 기준 오프셋 저장
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		FVector2D LocalWidgetPos = CanvasSlot->GetPosition();
		DragOffset = InMousePos - LocalWidgetPos;
	}
}

void UDesktopWindowBaseWidget::UpdateDrag(const FVector2D& InMousePos)
{
	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetPosition(InMousePos - DragOffset);
	}
}

void UDesktopWindowBaseWidget::StopDragging()
{
	bDragging = false;
}

bool UDesktopWindowBaseWidget::IsInTitleBar(const FVector2D& ScreenPos) const
{
	if (!TitleBar) return false;

	FGeometry Geometry = TitleBar->GetCachedGeometry();
	return Geometry.IsUnderLocation(ScreenPos);
}

void UDesktopWindowBaseWidget::SetAppInfo(const FName& InAppID, const FText& InAppName, UTexture2D* InIcon)
{
	AppID = InAppID;
	AppName = InAppName;
	AppIcon = InIcon;
}
