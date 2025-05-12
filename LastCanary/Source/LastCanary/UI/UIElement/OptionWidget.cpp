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
	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}

	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(0);
	}
}

void UOptionWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
}

void UOptionWidget::OnApplyButtonClicked()
{
	if (ULCOptionManager* OptionManager = GetGameInstance()->GetSubsystem<ULCOptionManager>())
	{
		OptionManager->ApplyOptions();
	}
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
