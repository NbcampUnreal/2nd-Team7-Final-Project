#include "UI/UIObject/ShopItemEntry.h"

#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "LastCanary.h"

void UShopItemEntry::NativeConstruct()
{
	Super::NativeConstruct();

	if (SelectButton)
	{
		DefaultButtonStyle = SelectButton->WidgetStyle;
		SelectButton->OnClicked.AddUniqueDynamic(this, &UShopItemEntry::OnSelectButtonClicked);
	}
}

void UShopItemEntry::NativeDestruct()
{
	Super::NativeDestruct();

	if (SelectButton)
	{
		SelectButton->OnClicked.RemoveDynamic(this, &UShopItemEntry::OnSelectButtonClicked);
	}
}

void UShopItemEntry::SetSelected(bool bInSelected)
{
	bIsSelected = bInSelected;

	if (SelectButton)
	{
		FButtonStyle NewStyle = DefaultButtonStyle;

		if (bIsSelected)
		{
			FSlateColor LockedColor = FSlateColor(FLinearColor::FromSRGBColor(FColor::FromHex("282828")));

			NewStyle.Normal.TintColor = LockedColor;

			SelectButton->SetStyle(NewStyle);
			SelectButton->SetIsEnabled(false);
		}
		else
		{
			SelectButton->SetStyle(DefaultButtonStyle); 
			SelectButton->SetIsEnabled(true);
		}
	}
}

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

void UShopItemEntry::OnSelectButtonClicked()
{
	LOG_Frame_WARNING(TEXT("ShopItemEntry Clicked: %s"), *ItemData.ItemName.ToString());

	if (OnItemClicked.IsBound())
	{
		OnItemClicked.Execute(this);
	}
}

const FItemDataRow& UShopItemEntry::GetItemData() const
{
	return ItemData;
}

int32 UShopItemEntry::GetItemID() const
{
	return ItemData.ItemID;
}
