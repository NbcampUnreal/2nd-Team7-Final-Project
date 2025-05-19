#include "UI/UIElement/InventoryMainWidget.h"
#include "UI/UIElement/ToolbarInventoryWidget.h"
#include "UI/UIElement/BackpackInventoryWidget.h"

#include "LastCanary.h"

void UInventoryMainWidget::InitializeInventory(UToolbarInventoryWidget* InToolbarWidget, UBackpackInventoryWidget* InBackpackWidget)
{
	InitializeToolbarInventory(InToolbarWidget);
	InitializeBackpackInventory(InBackpackWidget);
}

void UInventoryMainWidget::RefreshInventory()
{
	if (ToolbarWidget)
	{
		ToolbarWidget->RefreshInventoryUI();
	}

	if (BackpackWidget)
	{
		BackpackWidget->RefreshInventoryUI();
	}
}

void UInventoryMainWidget::InitializeToolbarInventory(UToolbarInventoryWidget* InToolbarWidget)
{
	if (!InToolbarWidget)
	{
		LOG_Item_WARNING(TEXT("[InventoryMainWidget::InitializeToolbarInventory] InToolbarWidget is null!"));
		return;
	}

	if (!ToolbarWidget)
	{
		LOG_Item_WARNING(TEXT("[InventoryMainWidget::InitializeToolbarInventory] ToolbarWidget is null!"));
		return;
	}

	ToolbarWidget->SetInventoryComponent(InToolbarWidget->GetInventoryComponent());
	ToolbarWidget->SetVisibility(ESlateVisibility::Visible);
}

void UInventoryMainWidget::InitializeBackpackInventory(UBackpackInventoryWidget* InBackpackWidget)
{
	if (BackpackWidget)
	{
		if (InBackpackWidget)
		{
			BackpackWidget->SetInventoryComponent(InBackpackWidget->GetInventoryComponent());
			BackpackWidget->SetVisibility(ESlateVisibility::Visible);
		}
		else
		{
			BackpackWidget->SetInventoryComponent(nullptr);
			BackpackWidget->SetVisibility(ESlateVisibility::Hidden);
		}
	}
}
