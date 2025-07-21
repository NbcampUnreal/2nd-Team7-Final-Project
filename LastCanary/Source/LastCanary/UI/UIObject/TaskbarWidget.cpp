#include "UI/UIObject/TaskbarWidget.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "UI/UIObject/TaskbarAppButton.h"
#include "Components/HorizontalBox.h"
#include "Components/HorizontalBoxSlot.h"
#include "Engine/Texture2D.h"

void UTaskbarWidget::RegisterApp(UUserWidget* AppWidget, const FName& AppID, const FText& AppName, UTexture2D* Icon)
{
	if (IsAppRegistered(AppID) || !AppSlotBox || !TaskbarAppButtonClass)
	{
		return;
	}

	UTaskbarAppButton* NewButton = CreateWidget<UTaskbarAppButton>(this, TaskbarAppButtonClass);
	if (NewButton)
	{
		NewButton->InitializeAppButton(AppID, AppName, Icon, AppWidget);
		AppSlotBox->AddChildToHorizontalBox(NewButton);
		ActiveAppButtons.Add(AppID, NewButton);
	}
}

void UTaskbarWidget::RemoveApp(const FName& AppID)
{
	if (UTaskbarAppButton* Button = ActiveAppButtons.FindRef(AppID))
	{
		Button->RemoveFromParent();
		ActiveAppButtons.Remove(AppID);
	}
}

bool UTaskbarWidget::IsAppRegistered(const FName& AppID) const
{
	return ActiveAppButtons.Contains(AppID);
}

void UTaskbarWidget::SetTaskbarAppButtonClass(TSubclassOf<UTaskbarAppButton> InClass)
{
	TaskbarAppButtonClass = InClass;
}

void UTaskbarWidget::AddAppButtonFor(UDesktopWindowBaseWidget* Window)
{
	if (!Window) return;

	FName AppID = Window->GetAppID();
	FText AppName = Window->GetAppName();
	UTexture2D* Icon = Window->GetAppIcon();

	if (!IsAppRegistered(AppID))
	{
		RegisterApp(Window, AppID, AppName, Icon);
	}
}
