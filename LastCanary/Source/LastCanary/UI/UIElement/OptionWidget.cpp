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
	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
	if (KeySettingTabButton)
	{
		KeySettingTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnKeySettingTabButtonClicked);
	}
	if (VoiceOptionTabButton)
	{
		VoiceOptionTabButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnKeyVoiceOptionTabButtonClicked);
	}
	if (GraphicsOptionButton)
	{
		GraphicsOptionButton->OnClicked.AddUniqueDynamic(this, &UOptionWidget::OnGraphicsSettingButtonClicked);
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
	if (GeneralTabButton)
	{
		GeneralTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnGeneralTabButtonClicked);
	}
	if (KeySettingTabButton)
	{
		KeySettingTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnKeySettingTabButtonClicked);
	}
	if (VoiceOptionTabButton)
	{
		VoiceOptionTabButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnKeyVoiceOptionTabButtonClicked);
	}
	if (GraphicsOptionButton)
	{
		GraphicsOptionButton->OnClicked.RemoveDynamic(this, &UOptionWidget::OnGraphicsSettingButtonClicked);
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
	//SetTabButtonStyle(GeneralTabButton, true);
	//SetTabButtonStyle(KeySettingTabButton, false);
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


	KeySettingWidget->InitialMappings();

	//SetTabButtonStyle(GeneralTabButton, false);
	//SetTabButtonStyle(KeySettingTabButton, true);

}

void UOptionWidget::OnKeyVoiceOptionTabButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(2);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Voice Option Widget is nullptr"));
	}
	//SetTabButtonStyle(GeneralTabButton, false);
	//SetTabButtonStyle(KeySettingTabButton, false);
}

void UOptionWidget::OnGraphicsSettingButtonClicked()
{
	if (OptionSwitcher)
	{
		OptionSwitcher->SetActiveWidgetIndex(3);
	}
	else
	{
		LOG_Frame_ERROR(TEXT("Graphics Widget is nullptr"));
	}
}

//void UOptionWidget::SetTabButtonStyle(UButton* Button, bool bIsSelected)
//{
//	if (!Button)
//	{
//		return;
//	}
//
//	FButtonStyle NewStyle = Button->WidgetStyle;
//
//	if (bIsSelected)
//	{
//		NewStyle.Normal.TintColor = FSlateColor(SelectedColor);
//		NewStyle.Hovered.TintColor = FSlateColor(SelectedColor * 0.95f);
//		NewStyle.Pressed.TintColor = FSlateColor(SelectedColor * 0.85f);
//	}
//	else
//	{
//		NewStyle.Normal.TintColor = FSlateColor(UnselectedColor);
//		NewStyle.Hovered.TintColor = FSlateColor(UnselectedColor * 1.1f);
//		NewStyle.Pressed.TintColor = FSlateColor(UnselectedColor * 0.9f);
//	}
//
//	Button->SetStyle(NewStyle);
//
//	if (Button == GeneralTabButton && GeneralTabText)
//	{
//		GeneralTabText->SetColorAndOpacity(
//			bIsSelected
//			? FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f)) 
//			: FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f)) 
//		);
//	}
//	else if (Button == KeySettingTabButton && KeySettingTabText)
//	{
//		KeySettingTabText->SetColorAndOpacity(
//			bIsSelected
//			? FSlateColor(FLinearColor(0.1f, 0.1f, 0.1f))
//			: FSlateColor(FLinearColor(0.9f, 0.9f, 0.9f))
//		);
//	}
//}
