#include "UI/UIObject/ShoppingCartWidget.h"
#include "UI/UIObject/ShoppingCartEntry.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

void UShoppingCartWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UShoppingCartWidget::NativeDestruct()
{
	Super::NativeDestruct();
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
		NewEntry->OnCartEntryRemoved.BindUObject(this, &UShoppingCartWidget::HandleRemoveCartEntry);

		CartItemBox->AddChild(NewEntry);
		CartEntries.Add(ItemData.ItemID, NewEntry);
	}

	RecalculateTotalPrice();
}

void UShoppingCartWidget::HandleRemoveCartEntry(UShoppingCartEntry* EntryToRemove)
{
	if (!EntryToRemove)
	{
		return;
	}

	int32 FoundKey = INDEX_NONE;
	for (const auto& Elem : CartEntries)
	{
		if (Elem.Value == EntryToRemove)
		{
			FoundKey = Elem.Key;
			break;
		}
	}

	if (FoundKey != INDEX_NONE)
	{
		CartEntries.Remove(FoundKey);
	}

	EntryToRemove->RemoveFromParent();
	RecalculateTotalPrice();
}

void UShoppingCartWidget::RecalculateTotalPrice()
{
	int32 Total = 0;

	for (const auto& Entry : CartEntries)
	{
		if (Entry.Value)
		{
			Total += Entry.Value->GetQuantity() * Entry.Value->GetItemPrice(); 
		}
	}

	if (TotalPriceText)
	{
		FString TotalText = FString::Printf(TEXT("총 가격: %d"), Total);
		TotalPriceText->SetText(FText::FromString(TotalText));
	}
}
