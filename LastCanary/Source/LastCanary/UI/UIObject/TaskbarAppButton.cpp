#include "UI/UIObject/TaskbarAppButton.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Blueprint/WidgetBlueprintLibrary.h"

void UTaskbarAppButton::InitializeAppButton(FName InAppID, const FText& InAppName, UTexture2D* InIcon, UUserWidget* InAppWidget)
{
	AppID = InAppID;
	AppWidget = InAppWidget;

	if (AppName)
	{
		AppName->SetText(InAppName);
	}
	if (AppIcon && InIcon)
	{
		AppIcon->SetBrushFromTexture(InIcon);
	}
}

void UTaskbarAppButton::NativeConstruct()
{
	Super::NativeConstruct();

	// 좌클릭 이벤트 바인딩
	UWidgetBlueprintLibrary::SetInputMode_UIOnlyEx(GetOwningPlayer(), this);
	SetKeyboardFocus();

	if (AppButton)
	{
		AppButton->OnClicked.AddUniqueDynamic(this, &UTaskbarAppButton::OnAppButtonClicked);
		// 향후 우클릭용 바인딩도 여기서 가능
	}
}

void UTaskbarAppButton::OnAppButtonClicked()
{
	if (AppWidget == nullptr)
	{
		return;
	}

	if (AppWidget->IsInViewport())
	{
		AppWidget->SetFocus();
		return;
	}

	AppWidget->AddToViewport(); // Optional: 높은 ZOrder
}

void UTaskbarAppButton::OnAppButtonRightClicked()
{
	// 우클릭으로 종료 등 처리하고 싶으면 여기에 구현
	// 예: UIManager->CloseApp(AppID);
}
