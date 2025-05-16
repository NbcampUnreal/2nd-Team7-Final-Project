#include "UI/UIObject/ShoppingCartEntry.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"

void UShoppingCartEntry::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShoppingCartEntry::InitCartEntry(const FItemDataRow& InItemData, int32 InQuantity)
{
	ItemData = InItemData;
	Quantity = InQuantity;

	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ItemData.ItemIcon);
	}
	if (ItemNameText)
	{
		ItemNameText->SetText(FText::FromName(ItemData.ItemName));
	}
	UpdateQuantity(Quantity); 
}

void UShoppingCartEntry::UpdateQuantity(int32 NewQuantity)
{
	Quantity = NewQuantity;

	if (QuantityText)
	{
		QuantityText->SetText(FText::AsNumber(Quantity));
	}
	if (TotalPriceText)
	{
		int32 Total = Quantity * ItemData.ItemPrice;
		TotalPriceText->SetText(FText::AsNumber(Total));
	}
}

int32 UShoppingCartEntry::GetQuantity() const
{
	return Quantity;
}

int32 UShoppingCartEntry::GetItemPrice() const
{
	return ItemData.ItemPrice;
}
