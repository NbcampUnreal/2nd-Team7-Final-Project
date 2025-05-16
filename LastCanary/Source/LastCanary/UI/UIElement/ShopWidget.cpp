#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/ShopItemEntry.h"
#include "UI/UIObject/ShopItemInfoWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"

#include "LastCanary.h"

void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();
	if (PurchaseButton)
	{
		PurchaseButton->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnPurchaseButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnExitButtonClicked);
	}
	if (ItemInfoWidget && ShoppingCartWidget)
	{
		ItemInfoWidget->SetShoppingCartWidget(ShoppingCartWidget);
	}
	if (ShopFadeAnim)
	{
		PlayAnimation(ShopFadeAnim, 0.0f, 1);
	}
	PopulateShopItems();
}

void UShopWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (PurchaseButton)
	{
		PurchaseButton->OnClicked.RemoveDynamic(this, &UShopWidget::OnPurchaseButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UShopWidget::OnExitButtonClicked);
	}
	if (ShopFadeAnim)
	{
		PlayAnimation(ShopFadeAnim, 0.0f, 1);
	}
}

void UShopWidget::PopulateShopItems()
{
	if (!ItemListBox || !ItemDataTable || !ShopItemEntryClass)
	{
		return;
	}

	ItemListBox->ClearChildren();

	TArray<FName> RowNames = ItemDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("Shop Load")))
		{
			if (!ItemData->bSellInShop)
			{
				continue;
			}

			UShopItemEntry* ItemEntry = CreateWidget<UShopItemEntry>(this, ShopItemEntryClass);
			if (ItemEntry)
			{
				ItemEntry->InitItem(*ItemData);
				ItemEntry->OnItemClicked.BindUObject(this, &UShopWidget::HandleItemClicked);
				ItemListBox->AddChild(ItemEntry);

				LOG_Frame_WARNING(TEXT("Shop item bound: %s"), *ItemData->ItemName.ToString());
			}
		}
	}
}

void UShopWidget::HandleItemClicked(const FItemDataRow& ClickedItem)
{
	LOG_Frame_WARNING(TEXT("HandleItemClicked called: %s"), *ClickedItem.ItemName.ToString());

	if (ItemInfoWidget == nullptr)
	{
		LOG_Frame_WARNING(TEXT("ItemInfoWidget is null"));
		return;
	}
	if (ItemDataTable== nullptr)
	{
		LOG_Frame_WARNING(TEXT("ItemDataTable is null"));
		return;
	}

	ItemInfoWidget->ItemDataTable = ItemDataTable;
	ItemInfoWidget->LoadItemFromDataTable(ClickedItem.ItemID);
}

void UShopWidget::OnPurchaseButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnPurchaseButtonClicked"));
}

void UShopWidget::OnExitButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnExitButtonClicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	if (UIManager)
	{
		UIManager->HideShopPopup();
	}
}
