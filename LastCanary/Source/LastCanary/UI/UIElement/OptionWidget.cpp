#include "UI/UIElement/OptionWidget.h"
#include "UI/UIObject/GeneralOptionWidget.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"

#include "Framework/GameInstance/LCOptionManager.h"

#include "LastCanary.h"

void UOptionWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (ApplyButton)
	{
		ApplyButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnApplyButtonClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnCloseButtonClicked);
	}
	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
	if (KeySettingTabButton)
	{
		KeySettingTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnKeySettingTabButtonClicked);
	}
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(0);
	}
}

void UOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (ApplyButton)
	{
		ApplyButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnApplyButtonClicked);
	}
	if (CloseButton)
	{
		CloseButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnCloseButtonClicked);
	}
	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
	if (KeySettingTabButton)
	{
		KeySettingTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnKeySettingTabButtonClicked);
	}
}

void UOptionWidget::OnApplyButtonClicked()
{
	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->ApplyOptions();
	}
}

void UOptionWidget::OnCloseButtonClicked()
{
	RemoveFromParent();
}

void UOptionWidget::OnGeneralTabButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(0);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("General Option Widget is nullptr"));
	}
}

void UOptionWidget::OnKeySettingTabButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(1);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Key Setting Widget is nullptr"));
	}
}
