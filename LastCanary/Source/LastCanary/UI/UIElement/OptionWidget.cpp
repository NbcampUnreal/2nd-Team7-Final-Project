#include "UI/UIElement/OptionWidget.h"
#include "UI/UIObject/GeneralOptionWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Components/WidgetSwitcher.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

#include "Framework/GameInstance/LCOptionManager.h"

#include "LastCanary.h"

#include "UI/UIObject/KeySettingWidget.h"
#include "UI/UIObject/GraphicsSettingPanel.h"

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
	if (ResetButton)
	{
		ResetButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnResetButtonClicked);
	}

	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
	if (GraphicsOptionButton)
	{
		GraphicsOptionButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnGraphicsSettingButtonClicked);
	}
	if (KeySettingTabButton)
	{
		KeySettingTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnKeySettingTabButtonClicked);
	}
	if (VoiceOptionTabButton)
	{
		VoiceOptionTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnVoiceOptionTabButtonClicked);
	}

	if (OptionSwitcher)
	{
		OnGeneralTabButtonClicked();
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
	if (ResetButton)
	{
		ResetButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnResetButtonClicked);
	}

	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
	if (GraphicsOptionButton)
	{
		GraphicsOptionButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnGraphicsSettingButtonClicked);
	}
	if (KeySettingTabButton)
	{
		KeySettingTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnKeySettingTabButtonClicked);
	}
	if (VoiceOptionTabButton)
	{
		VoiceOptionTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnVoiceOptionTabButtonClicked);
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
	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		UIManager->HideOptionWidget();
	}
}

void UOptionWidget::OnResetButtonClicked()
{
	//TODO : 초기화 기능 구현
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

void UOptionWidget::OnGraphicsSettingButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(1);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Graphics Widget is nullptr"));
	}
}

void UOptionWidget::OnKeySettingTabButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(2);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Key Setting Widget is nullptr"));
	}

	KeySettingWidget->InitialMappings();
}

void UOptionWidget::OnVoiceOptionTabButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(3);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Voice Option Widget is nullptr"));
	}
}
