#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/ShopItemEntry.h"
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

	PopulateShopItems();
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
				ItemListBox->AddChild(ItemEntry);
			}
		}
	}
}

void UShopWidget::HandleItemClicked(const FItemDataRow& ClickedItem)
{
	LOG_Frame_WARNING(TEXT("Clicked Item: %s"), *ClickedItem.ItemName.ToString());
}

void UShopWidget::OnPurchaseButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnPurchaseButtonClicked"));
}
