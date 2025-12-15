#include "UI/UIObject/TaskbarAppButton.h"

#include "Components/Image.h"
#include "Components/Button.h"

#include "UI/UIObject/DesktopWindowBaseWidget.h"

#include "UI/Manager/LCUIManager.h"
#include "UI/Manager/LCDesktopWindowManager.h"

void UTaskbarAppButton::InitializeAppButton(UTexture2D* InIcon, UUserWidget* InAppWidget)
{
	AppWidget = InAppWidget;

	if (AppIcon && InIcon)
	{
		AppIcon->SetBrushFromTexture(InIcon);
	}
}

void UTaskbarAppButton::NativeConstruct()
{
	Super::NativeConstruct();

	if (AppButton)
	{
		AppButton->OnClicked.AddUniqueDynamic(this, &UTaskbarAppButton::OnAppButtonClicked);
		AppButton->OnHovered.AddUniqueDynamic(this, &UTaskbarAppButton::OnAppButtonRightClicked);
	}
}

void UTaskbarAppButton::OnAppButtonClicked()
{
	if (AppWidget == nullptr)
	{
		return;
	}

	if (UDesktopWindowBaseWidget* Window = Cast<UDesktopWindowBaseWidget>(AppWidget))
	{
		ULCDesktopWindowManager* WindowManager = ResolveUIManager()->GetDesktopWindowManager();
		if (WindowManager == nullptr)
		{
			return;
		}

		// 최소화 상태면 복원
		if (Window->IsMinimized())
		{
			Window->SetMinimized(false);
			Window->PlayRestoreAnimation();

			// 다시 데스크탑에 추가
			WindowManager->OpenWindow(Window);
			return;
		}
		else
		{
			// 이미 맨 위면 최소화
			if (WindowManager->IsTopMost(Window))
			{
				Window->SetMinimized(true);
				Window->PlayMinimizeAnimation();
				return;
			}

			// 맨 위로 올리기
			WindowManager->OpenWindow(Window);
			Window->SetFocus();
			return;
		}
	}

	// UDesktopWindowBaseWidget이 아니라면, 일단 Viewport에 추가 (호환성용)
	if (AppWidget->IsInViewport() == false)
	{
		AppWidget->AddToViewport(ResolveUIManager()->GetDesktopWindowManager()->GetNextZOrder());
	}
	AppWidget->SetFocus();
}

void UTaskbarAppButton::OnAppButtonRightClicked()
{
	// 우클릭으로 종료 등 처리하고 싶으면 여기에 구현
	// 예: UIManager->CloseApp(AppID);
}
