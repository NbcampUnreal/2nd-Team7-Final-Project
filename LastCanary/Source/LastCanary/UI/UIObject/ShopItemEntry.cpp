#include "UI/UIObject/ShopItemEntry.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "LastCanary.h"

void UShopItemEntry::InitItem(const FItemDataRow& InItemData)
{
	ItemData = InItemData;

	if (ItemIcon)
	{
		ItemIcon->SetBrushFromTexture(ItemData.ItemIcon);
	}
	if (ItemNameText) 
	{
		ItemNameText->SetText(FText::FromName(ItemData.ItemName));
	}
	if (ItemPriceText) 
	{
		ItemPriceText->SetText(FText::AsNumber(ItemData.ItemPrice));
	}
}

void UShopItemEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectButton)
	{
		SelectButton->OnClicked.AddUniqueDynamic(this, &UShopItemEntry::OnSelectButtonClicked);
	}
}

void UShopItemEntry::OnSelectButtonClicked()
{
	LOG_Frame_WARNING(TEXT("ShopItemEntry Clicked: %s"), *ItemData.ItemName.ToString());

	if (OnItemClicked.IsBound())
	{
		OnItemClicked.Execute(ItemData);
	}
}