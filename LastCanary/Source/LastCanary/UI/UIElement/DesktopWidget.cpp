#include "UI/UIElement/DesktopWidget.h"
#include "UI/UIElement/ShopWidget.h"
#include "UI/UIElement/CharacterCustomizationWidget.h"

#include "UI/UIObject/TaskbarWidget.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"

#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Button.h"

#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCDesktopWindowManager.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Kismet/GameplayStatics.h"

//-----------------
// 초기화 및 구성
//-----------------
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
	if (CharacterCustomizationIconButton)
	{
		CharacterCustomizationIconButton->OnClicked.AddUniqueDynamic(this, &UDesktopWidget::OnCharacterCustomizationIconSingleClicked);
	}

	ClickCount = 0;
	ResetAllAppButtonHighlights();
}

void UDesktopWidget::PowerOn()
{
	ResetAllAppButtonHighlights();

	SetVisibility(ESlateVisibility::Visible);

	if (PlayPowerOnAnim)
	{
		PlayAnimation(PlayPowerOnAnim);
	}
}

//-----------------
// 클릭 처리
//-----------------
FReply UDesktopWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		ResetAllAppButtonHighlights();
	}

	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
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

//-----------------
// 아이콘 클릭 이벤트
//-----------------
void UDesktopWidget::OnShopIconSingleClicked()
{
	ClickCount++;

	if (ClickCount == 1)
	{
		HighlightAppButton(ShopIconButton);
		GetWorld()->GetTimerManager().SetTimer(
			DoubleClickTimerHandle,
			this,
			&UDesktopWidget::ResetClickCount,
			0.3f,
			false
		);
	}
	else if (ClickCount == 2)
	{
		GetWorld()->GetTimerManager().ClearTimer(DoubleClickTimerHandle);
		ResetClickCount();
		HandleShopAppLaunch();
	}
}

void UDesktopWidget::ResetClickCount()
{
	ClickCount = 0;
}

void UDesktopWidget::HighlightAppButton(UButton* TargetButton)
{
	if (CurrentlyHighlightedButton && CurrentlyHighlightedButton != TargetButton)
	{
		ResetAppButtonHighlight(CurrentlyHighlightedButton);
	}

	if (TargetButton)
	{
		FButtonStyle Style = TargetButton->WidgetStyle;
		Style.Normal.TintColor = FSlateColor(FLinearColor::Yellow);
		Style.Hovered.TintColor = FSlateColor(FLinearColor::Yellow);
		Style.Pressed.TintColor = FSlateColor(FLinearColor::Yellow);
		TargetButton->SetStyle(Style);
		CurrentlyHighlightedButton = TargetButton;
	}
}

void UDesktopWidget::ResetAppButtonHighlight(UButton* Button)
{
	if (Button == nullptr)
	{
		return;
	}

	FButtonStyle Style = Button->WidgetStyle;
	Style.Normal.TintColor = FSlateColor::UseForeground();
	Style.Hovered.TintColor = FSlateColor::UseForeground();
	Style.Pressed.TintColor = FSlateColor::UseForeground();
	Button->SetStyle(Style);
}

void UDesktopWidget::ResetAllAppButtonHighlights()
{
	ResetAppButtonHighlight(CurrentlyHighlightedButton);
	CurrentlyHighlightedButton = nullptr;
}

//-----------------
// 어플들 실행 처리
//-----------------
void UDesktopWidget::HandleShopAppLaunch()
{
	ResetAllAppButtonHighlights();

	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(PC))
		{
			ULCUIManager* UIManager = RoomPC->GetUIManager();
			if (UIManager == nullptr)
			{
				return;
			}

			RoomPC->Server_ShowShopWidget();
			if (UShopWidget* ShopWidget = UIManager->GetCachedShopWidget())
			{
				if (ULCDesktopWindowManager* LCDesktopWindowManager = UIManager->GetDesktopWindowManager())
				{
					if (ShopWidget->IsMinimized())
					{
						ShopWidget->SetVisibility(ESlateVisibility::Visible);
						ShopWidget->SetMinimized(false);
						ShopWidget->PlayRestoreAnimation();

					}
					WindowManager->OpenWindow(ShopWidget);
				}
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

void UDesktopWidget::OnCharacterCustomizationIconSingleClicked()
{
	ClickCount++;

	if (ClickCount == 1)
	{
		HighlightAppButton(CharacterCustomizationIconButton);
		GetWorld()->GetTimerManager().SetTimer(
			DoubleClickTimerHandle,
			this,
			&UDesktopWidget::ResetClickCount,
			0.3f,
			false
		);
	}
	else if (ClickCount == 2)
	{
		GetWorld()->GetTimerManager().ClearTimer(DoubleClickTimerHandle);
		ResetClickCount();
		HandleCharacterCustomizationLaunch();
	}
}

void UDesktopWidget::HandleCharacterCustomizationLaunch()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		if (ALCRoomPlayerController* RoomPC = Cast<ALCRoomPlayerController>(PC))
		{
			if (ULCUIManager* UIManager = RoomPC->GetUIManager())
			{
				UCharacterCustomizationWidget* Widget = UIManager->ShowCharacterCustomizationWidget();

				if (Widget->IsMinimized())
				{
					Widget->SetVisibility(ESlateVisibility::Visible);
					Widget->SetMinimized(false);
					Widget->PlayRestoreAnimation();
				}
			}
		}
	}
}

//-----------------
// 데스크탑 닫기 버튼
//-----------------
void UDesktopWidget::OnCloseDesktopClicked()
{
	ResetAllAppButtonHighlights();

	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		UIManager->HideDesktop();
	}
}

//-----------------
// RoomPC 접근
//-----------------
ALCRoomPlayerController* UDesktopWidget::GetRoomPC() const
{
	return Cast<ALCRoomPlayerController>(UGameplayStatics::GetPlayerController(this, 0));
}

//-----------------
// 창 추가
//-----------------
void UDesktopWidget::AddWindow(UUserWidget* NewWindow)
{
	ResetAllAppButtonHighlights();

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

	if (NewWindow->GetParent() == nullptr)
	{
		if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(WindowContainer->AddChild(NewWindow)))
		{
			CanvasSlot->SetAutoSize(false);
			CanvasSlot->SetAnchors(FAnchors(0.f, 0.f));
			CanvasSlot->SetAlignment(FVector2D(0.f, 0.f));
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
