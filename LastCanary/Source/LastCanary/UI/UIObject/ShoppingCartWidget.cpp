#include "UI/UIObject/ShoppingCartWidget.h"
#include "UI/UIObject/ShoppingCartEntry.h"

#include "Components/ScrollBox.h"

void UShoppingCartWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShoppingCartWidget::AddItemToCart(const FItemDataRow& ItemData, int32 Quantity)
{
	if (!CartItemBox || !ShoppingCartEntryClass)
	{
		UE_LOG(LogTemp, Warning, TEXT("ShoppingCartWidget: Missing widget references"));
		return;
	}

	if (CartEntries.Contains(ItemData.ItemID))
	{
		UShoppingCartEntry* ExistingEntry = CartEntries[ItemData.ItemID];
		int32 NewQuantity = ExistingEntry->GetQuantity() + Quantity;
		ExistingEntry->UpdateQuantity(NewQuantity);
	}
	else
	{
		UShoppingCartEntry* NewEntry = CreateWidget<UShoppingCartEntry>(this, ShoppingCartEntryClass);
		if (!NewEntry)
		{
			return;
		}

		NewEntry->InitCartEntry(ItemData, Quantity);
		CartItemBox->AddChild(NewEntry);
		CartEntries.Add(ItemData.ItemID, NewEntry);
	}
}
