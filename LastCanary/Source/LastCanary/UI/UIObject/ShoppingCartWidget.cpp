#include "UI/UIObject/ShoppingCartWidget.h"
#include "UI/UIObject/ShoppingCartEntry.h"
#include "DataTable/ItemDataRow.h"
#include "Item/ItemBase.h"

#include "Components/ScrollBox.h"
#include "Components/TextBlock.h"

#include "LastCanary.h"

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

	TotalPrice = Total;

	UpdateVisuals();

	// 구매 가능 여부에 따라 델리게이트 실행
	const bool bCanAfford = PlayerGold >= TotalPrice;
	OnCartValidityChanged.ExecuteIfBound(bCanAfford);
}

void UShoppingCartWidget::SetPlayerGold(int32 NewGold)
{
	PlayerGold = NewGold;
	UpdateVisuals();
}

void UShoppingCartWidget::UpdateVisuals()
{
	if (TotalPriceText)
	{
		FString PriceString = FString::Printf(TEXT("총 금액: %d G"), TotalPrice);
		TotalPriceText->SetText(FText::FromString(PriceString));

		const bool bCanAfford = PlayerGold >= TotalPrice;
		FSlateColor Color = bCanAfford ? FSlateColor(FLinearColor::White) : FSlateColor(FLinearColor::Red);
		TotalPriceText->SetColorAndOpacity(Color);
	}

	if (PlayerGoldText)
	{
		FString GoldString = FString::Printf(TEXT("보유 골드: %d G"), PlayerGold);
		PlayerGoldText->SetText(FText::FromString(GoldString));
	}
}

TArray<FItemDropData> UShoppingCartWidget::GetItemDropList() const
{
	TArray<FItemDropData> Result;

	for (const TPair<int32, UShoppingCartEntry*>& Pair : CartEntries)
	{
		if (const UShoppingCartEntry* Entry = Pair.Value)
		{
			FItemDropData DropData;
			DropData.ItemClass = Entry->GetItemData().ItemActorClass; // FItemDataRow에 ActorClass가 있어야 함
			DropData.Count = Entry->GetQuantity(); // Entry가 수량을 저장하고 있어야 함
			DropData.ItemID = Entry->GetItemData().ItemID;

			if (DropData.ItemClass && DropData.Count > 0)
			{
				Result.Add(DropData);
				LOG_Frame_WARNING(TEXT("ItemDataCount : %d"), DropData.Count);
			}
		}
	}

	return Result;
}

void UShoppingCartWidget::ClearCart()
{
	if (CartItemBox)
	{
		CartItemBox->ClearChildren();
	}

	CartEntries.Empty();
	RecalculateTotalPrice();
}
