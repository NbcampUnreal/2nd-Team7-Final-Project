#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "Components/Image.h"
#include "Components/Button.h"
#include "Components/HorizontalBox.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIObject/TaskbarWidget.h"
#include "UI/UIElement/DesktopWidget.h"
#include "UI/Manager/LCDesktopWindowManager.h"

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

	UpdateMaximizeButtonIcon();
}

void UDesktopWindowBaseWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		if (UTaskbarWidget* Taskbar = UIManager->GetTaskbarWidget())
		{
			Taskbar->RemoveApp(AppID);
		}
	}
}

void UDesktopWindowBaseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bDragging)
	{
		const TSet<FKey>& PressedButtons = FSlateApplication::Get().GetPressedMouseButtons();

		if (!PressedButtons.Contains(EKeys::LeftMouseButton))
		{
			StopDragging();
			return;
		}

		UpdateDrag(FSlateApplication::Get().GetCursorPos());
	}

	if (!bDragging)
	{
		bWasMouseOutsideWindowContainer = false;
		return;
	}

	const FVector2D MousePos = FSlateApplication::Get().GetCursorPos();

	if (UDesktopWidget* Desktop = GetDesktopWidget())
	{
		if (UCanvasPanel* WindowContainer = Desktop->GetWindowContainer())
		{
			const FGeometry Geometry = WindowContainer->GetCachedGeometry();
			const bool bIsNowInside = Geometry.IsUnderLocation(MousePos);

			if (bWasMouseOutsideWindowContainer && bIsNowInside)
			{
				if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
				{
					const FGeometry WidgetGeometry = GetCachedGeometry();
					const FVector2D WidgetScreenPos = WidgetGeometry.LocalToAbsolute(FVector2D::ZeroVector);
					const FVector2D TargetScreenPos = MousePos - DragOffset;

					const FVector2D NewLocalPos = Geometry.AbsoluteToLocal(TargetScreenPos);
					CanvasSlot->SetPosition(NewLocalPos);
				}
			}

			bWasMouseOutsideWindowContainer = !bIsNowInside;
		}
	}
}

void UDesktopWindowBaseWidget::InitDesktopWindow()
{
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
	if (WindowManager)
	{
		WindowManager->MinimizeWindow(this);
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UDesktopWindowBaseWidget::CloseWindow()
{
	if (WindowManager)
	{
		WindowManager->CloseWindow(this);
	}
	else
	{
		RemoveFromParent();
	}
}

void UDesktopWindowBaseWidget::ToggleMaximizeRestore()
{
	if (WindowManager)
	{
		WindowManager->ToggleMaximizeRestore(this);
		UpdateMaximizeButtonIcon();
		return;
	}

	//UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot);
	//if (!CanvasSlot) return;

	//if (bIsMaximized)
	//{
	//	CanvasSlot->SetAnchors(FAnchors(0.5f));
	//	CanvasSlot->SetAlignment(FVector2D(0.f));
	//	CanvasSlot->SetOffsets(FMargin(OriginalPosition.X, OriginalPosition.Y, OriginalSize.X, OriginalSize.Y));
	//	bIsMaximized = false;
	//}
	//else
	//{
	//	FMargin Offsets = CanvasSlot->GetOffsets();
	//	OriginalPosition = FVector2D(Offsets.Left, Offsets.Top);
	//	OriginalSize = FVector2D(Offsets.Right, Offsets.Bottom);

	//	CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));
	//	CanvasSlot->SetAlignment(FVector2D(0.f));
	//	CanvasSlot->SetOffsets(FMargin(0.f));
	//	bIsMaximized = true;
	//}
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
		// Bring to front (단, 데스크탑 자체는 제외)
		if (!Cast<UDesktopWidget>(this) && IsInViewport())
		{
			RemoveFromViewport();
			const int32 NewZOrder = ResolveUIManager()->GetDesktopWindowManager()->GetNextZOrder();
			AddToViewport(NewZOrder);
		}

		const FVector2D CurrentPos = InMouseEvent.GetScreenSpacePosition();
		const FDateTime CurrentTime = FDateTime::Now();
		const float Elapsed = (CurrentTime - LastClickTime).GetTotalSeconds();

		// 더블 클릭 → 최대화 / 복원 토글
		/*if (Elapsed < DoubleClickThreshold && FVector2D::Distance(CurrentPos, LastClickPosition) < MaxClickDelta)
		{
			ToggleMaximizeRestore();
			LastClickTime = FDateTime(0);
			LastClickPosition = FVector2D::ZeroVector;
			return FReply::Handled();
		}
		else
		{*/
			// 드래그 시작
			StartDragging(CurrentPos);
			LastClickTime = CurrentTime;
			LastClickPosition = CurrentPos;
			return FReply::Handled();
		// }
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

void UDesktopWindowBaseWidget::StartDragging(const FVector2D& InMouseScreenPos)
{
	if (bIsMaximized)
	{
		return;
	}

	bDragging = true;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		const FGeometry Geometry = GetCachedGeometry();
		const FVector2D WidgetScreenPos = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
		DragOffset = InMouseScreenPos - WidgetScreenPos;
	}
}

void UDesktopWindowBaseWidget::UpdateDrag(const FVector2D& InMouseScreenPos)
{
	if (bDragging == false)
	{
		return;
	}

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		if (UCanvasPanel* WindowContainer = Cast<UCanvasPanel>(GetParent()))
		{
			const FGeometry ContainerGeometry = WindowContainer->GetCachedGeometry();
			const FVector2D TargetScreenPos = InMouseScreenPos - DragOffset;

			const FVector2D ContainerSize = ContainerGeometry.GetLocalSize();
			const FVector2D MouseLocalPos = ContainerGeometry.AbsoluteToLocal(InMouseScreenPos);

			const float TaskbarHeight = 50.f;

			const FVector2D ClampedMouseLocalPos = FVector2D(
				FMath::Clamp(MouseLocalPos.X, 0.0f, ContainerSize.X),
				FMath::Clamp(MouseLocalPos.Y, 0.0f, ContainerSize.Y - TaskbarHeight)
			);

			const FVector2D OffsetFromClamped = InMouseScreenPos - ContainerGeometry.LocalToAbsolute(ClampedMouseLocalPos);
			const FVector2D FinalScreenPos = InMouseScreenPos - OffsetFromClamped;
			const FVector2D FinalLocalPos = ContainerGeometry.AbsoluteToLocal(FinalScreenPos - DragOffset);

			CanvasSlot->SetPosition(FinalLocalPos);
		}
	}
}

void UDesktopWindowBaseWidget::StopDragging()
{
	bDragging = false;
}

bool UDesktopWindowBaseWidget::IsInTitleBar(const FVector2D& ScreenPos) const
{
	if (TitleBar == nullptr)
	{
		return false;
	}

	const FGeometry& TitleGeometry = TitleBar->GetCachedGeometry();
	return TitleGeometry.IsUnderLocation(ScreenPos);
}

UDesktopWidget* UDesktopWindowBaseWidget::GetDesktopWidget() const
{
	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		return UIManager->GetDesktopWidget();
	}
	return nullptr;
}

void UDesktopWindowBaseWidget::SetAppInfo(const FName& InAppID, const FText& InAppName, UTexture2D* InIcon)
{
	AppID = InAppID;
	AppName = InAppName;
	AppIcon = InIcon;
}

bool UDesktopWindowBaseWidget::IsMinimized() const
{
	return bIsMinimized;
}

void UDesktopWindowBaseWidget::SetMinimized(bool bInIsMinimized)
{
	bIsMinimized = bInIsMinimized;
}

bool UDesktopWindowBaseWidget::IsMaximized() const
{
	return bIsMaximized;
}

void UDesktopWindowBaseWidget::SetMaximized(bool bInMaximized)
{
	bIsMaximized = bInMaximized;
	UpdateMaximizeButtonIcon(); 
}

void UDesktopWindowBaseWidget::SetOriginalPosition(const FVector2D& InPos)
{
	OriginalPosition = InPos;
}

void UDesktopWindowBaseWidget::SetOriginalSize(const FVector2D& InSize)
{
	OriginalSize = InSize;
}

const FVector2D UDesktopWindowBaseWidget::GetOriginalPosition() const
{
	return OriginalPosition;
}

const FVector2D UDesktopWindowBaseWidget::GetOriginalSize() const
{
	return OriginalSize;
}

FName UDesktopWindowBaseWidget::GetAppID() const
{
	return AppID;
}

FText UDesktopWindowBaseWidget::GetAppName() const
{
	return AppName;
}

UTexture2D* UDesktopWindowBaseWidget::GetAppIcon() const
{
	return AppIcon;
}

void UDesktopWindowBaseWidget::SetWindowManager(ULCDesktopWindowManager* InManager)
{
	WindowManager = InManager;
}

void UDesktopWindowBaseWidget::PlayMinimizeAnimation()
{
	if (MinimizeAnim)
	{
		PlayAnimation(MinimizeAnim, 0.f, 1, EUMGSequencePlayMode::Forward);
	}

}

void UDesktopWindowBaseWidget::PlayRestoreAnimation()
{
	if (MinimizeAnim)
	{
		AddToViewport(ResolveUIManager()->GetDesktopWindowManager()->GetNextZOrder());
		PlayAnimation(MinimizeAnim, 0.f, 1, EUMGSequencePlayMode::Reverse);
	}
}

void UDesktopWindowBaseWidget::AnimateWindowTransform(bool bMaximize, FVector2D InTargetSize)
{
	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(this->Slot))
	{
		bAnimatingToMaximized = bMaximize;
		AnimationAlpha = 0.0f;
		ElapsedTime = 0.0f;

		StartPosition = CanvasPanelSlot->GetPosition();
		StartSize = CanvasPanelSlot->GetSize();

		if (bMaximize)
		{
			TargetPosition = FVector2D(0.0f, 0.0f);
			TargetSize = InTargetSize;
		}
		else
		{
			TargetPosition = GetOriginalPosition();
			TargetSize = GetOriginalSize();
		}

		GetWorld()->GetTimerManager().SetTimer(
			WindowTransformAnimTimer,
			this,
			&UDesktopWindowBaseWidget::UpdateWindowTransformAnimation,
			0.01f,
			true
		);
	}
}

void UDesktopWindowBaseWidget::UpdateWindowTransformAnimation()
{
	ElapsedTime += 0.01f;
	AnimationAlpha = FMath::Clamp(ElapsedTime / AnimationDuration, 0.0f, 1.0f);

	if (UCanvasPanelSlot* CanvasPanelSlot = Cast<UCanvasPanelSlot>(this->Slot))
	{
		FVector2D NewPos = FMath::Lerp(StartPosition, TargetPosition, AnimationAlpha);
		FVector2D NewSize = FMath::Lerp(StartSize, TargetSize, AnimationAlpha);

		CanvasPanelSlot->SetPosition(NewPos);
		CanvasPanelSlot->SetSize(NewSize);
	}

	if (AnimationAlpha >= 1.0f)
	{
		GetWorld()->GetTimerManager().ClearTimer(WindowTransformAnimTimer);

		if (bAnimatingToMaximized)
		{
			SetMaximized(true);
		}
		else
		{
			SetMaximized(false);
		}
	}
}

void UDesktopWindowBaseWidget::UpdateMaximizeButtonIcon()
{
	if (MaximizeButton == nullptr)
	{
		return;
	}

	FSlateColor NormalColor = FSlateColor(FLinearColor(0.495466f, 0.495466f, 0.495466f, 1.0f));
	FSlateColor HoveredColor = FSlateColor(FLinearColor(1.0f, 1.0f, 1.0f, 1.0f));
	FSlateColor PressedColor = FSlateColor(FLinearColor(0.384266f, 0.384266f, 0.384266f, 1.0f));

	if (bIsMaximized)
	{
		MaximizeButton->WidgetStyle.Normal = RestoreBrush;
		MaximizeButton->WidgetStyle.Hovered = RestoreBrush;
		MaximizeButton->WidgetStyle.Pressed = RestoreBrush;
	}
	else
	{
		MaximizeButton->WidgetStyle.Normal = MaximizeBrush;
		MaximizeButton->WidgetStyle.Hovered = MaximizeBrush;
		MaximizeButton->WidgetStyle.Pressed = MaximizeBrush;
	}

	MaximizeButton->WidgetStyle.Normal.TintColor = NormalColor;
	MaximizeButton->WidgetStyle.Hovered.TintColor = HoveredColor;
	MaximizeButton->WidgetStyle.Pressed.TintColor = PressedColor;
}
