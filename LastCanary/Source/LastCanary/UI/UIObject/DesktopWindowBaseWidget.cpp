#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "Components/Button.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "UI/Manager/LCUIManager.h"
#include "UI/UIObject/TaskbarWidget.h"
#include "UI/UIElement/DesktopWidget.h"

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

void UDesktopWindowBaseWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bDragging)
	{
		const TSet<FKey>& PressedButtons = FSlateApplication::Get().GetPressedMouseButtons();

		// 드래그 중인데 마우스 왼쪽 버튼이 더 이상 눌려있지 않다면 → 끊긴 것임
		if (!PressedButtons.Contains(EKeys::LeftMouseButton))
		{
			StopDragging(); // 이게 핵심
			return;
		}

		// 정상 드래그 처리
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

			// 1. 이전엔 밖, 지금은 안 → 순간이동
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

			// 2. 현재 상태 저장
			bWasMouseOutsideWindowContainer = !bIsNowInside;
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
		// 복원
		CanvasSlot->SetAnchors(FAnchors(0.5f, 0.5f, 0.5f, 0.5f));
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
		CanvasSlot->SetOffsets(FMargin(OriginalPosition.X, OriginalPosition.Y, OriginalSize.X, OriginalSize.Y));

		bIsMaximized = false;
	}
	else
	{
		// 최대화
		OriginalPosition = CanvasSlot->GetPosition();
		OriginalSize = CanvasSlot->GetSize();

		CanvasSlot->SetAnchors(FAnchors(0.f, 0.f, 1.f, 1.f));     // 전체 창 기준
		CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));            // 좌측 상단 정렬
		CanvasSlot->SetOffsets(FMargin(0.f));                     // 전체 영역 차지

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

UDesktopWidget* UDesktopWindowBaseWidget::GetDesktopWidget() const
{
	ULCUIManager* UIManager = ResolveUIManager();
	if (UIManager)
	{
		return UIManager->GetDesktopWidget();
	}
	return nullptr;
}

void UDesktopWindowBaseWidget::StartDragging(const FVector2D& InMouseScreenPos)
{
	bDragging = true;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		// CanvasSlot 위치는 Local → Viewport 좌표계로 변환 필요
		FGeometry Geometry = GetCachedGeometry();
		const FVector2D WidgetScreenPos = Geometry.LocalToAbsolute(FVector2D::ZeroVector);
		DragOffset = InMouseScreenPos - WidgetScreenPos;
	}
}

void UDesktopWindowBaseWidget::UpdateDrag(const FVector2D& InMouseScreenPos)
{
	if (!bDragging) return;

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		// 데스크탑에서 벗어났는지 검사
		if (UDesktopWidget* Desktop = GetDesktopWidget())
		{
			if (UCanvasPanel* WindowContainer = Desktop->GetWindowContainer())
			{
				const FGeometry ContainerGeometry = WindowContainer->GetCachedGeometry();
				if (!ContainerGeometry.IsUnderLocation(InMouseScreenPos))
				{
					return;
				}
			}
		}

		// 부모(CanvasPanel)의 Geometry 기준으로 Local 위치 계산
		if (UCanvasPanel* WindowContainer = Cast<UCanvasPanel>(GetParent()))
		{
			const FGeometry ContainerGeometry = WindowContainer->GetCachedGeometry();

			// 위젯의 최종 위치는: 마우스 현재 스크린 위치 - 드래그 시작 당시 Offset
			const FVector2D TargetScreenPos = InMouseScreenPos - DragOffset;

			// → 이 위치를 부모(Canvas)의 Local로 변환
			const FVector2D NewLocalPos = ContainerGeometry.AbsoluteToLocal(TargetScreenPos);

			CanvasSlot->SetPosition(NewLocalPos);
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

	FGeometry Geometry = TitleBar->GetCachedGeometry();

	// Screen → Local 변환
	const FVector2D LocalPos = Geometry.AbsoluteToLocal(ScreenPos);

	// 실제 Local 영역 안에 있는지 판단
	return Geometry.GetLocalSize().X > 0.f && Geometry.GetLocalSize().Y > 0.f &&
		LocalPos.X >= 0.f && LocalPos.X <= Geometry.GetLocalSize().X &&
		LocalPos.Y >= 0.f && LocalPos.Y <= Geometry.GetLocalSize().Y;
}

void UDesktopWindowBaseWidget::SetAppInfo(const FName& InAppID, const FText& InAppName, UTexture2D* InIcon)
{
	AppID = InAppID;
	AppName = InAppName;
	AppIcon = InIcon;
}
