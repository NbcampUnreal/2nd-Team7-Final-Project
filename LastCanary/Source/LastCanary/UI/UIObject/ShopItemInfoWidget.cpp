#include "UI/UIObject/ShopItemInfoWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "Engine/DataTable.h"
#include "DataTable/ItemDataRow.h"

#include "LastCanary.h"


void UShopItemInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (IncreaseButton)
	{
		IncreaseButton->OnClicked.AddUniqueDynamic(this, &UShopItemInfoWidget::OnIncreaseButtonClicked);
	}
	if (DecreaseButton)
	{
		DecreaseButton->OnClicked.AddUniqueDynamic(this, &UShopItemInfoWidget::OnDecreaseButtonClicked);
	}
	if (AddToCartButton)
	{
		AddToCartButton->OnClicked.AddUniqueDynamic(this, &UShopItemInfoWidget::OnAddToCartButtonClicked);
	}

	UpdateCountDisplay();
}

void UShopItemInfoWidget::NativeDestruct()
{
	Super::NativeDestruct();
	if (IncreaseButton)
	{
		IncreaseButton->OnClicked.RemoveDynamic(this, &UShopItemInfoWidget::OnIncreaseButtonClicked);
	}
	if (DecreaseButton)
	{
		DecreaseButton->OnClicked.RemoveDynamic(this, &UShopItemInfoWidget::OnDecreaseButtonClicked);
	}
	if (AddToCartButton)
	{
		AddToCartButton->OnClicked.RemoveDynamic(this, &UShopItemInfoWidget::OnAddToCartButtonClicked);
	}
}

void UShopItemInfoWidget::OnIncreaseButtonClicked()
{
	SelectedCount++;
	UpdateCountDisplay();
}

void UShopItemInfoWidget::OnDecreaseButtonClicked()
{
	if (SelectedCount > 1)
	{
		SelectedCount--;
		UpdateCountDisplay();
	}
}

void UShopItemInfoWidget::UpdateCountDisplay()
{
	if (CountText)
	{
		CountText->SetText(FText::AsNumber(SelectedCount));
	}
}

void UShopItemInfoWidget::OnAddToCartButtonClicked()
{
	// 장바구니에 항목 추가 로직
	LOG_Frame_WARNING(TEXT("AddToCart: Count = %d, Price = %d, Total = %d"), SelectedCount, ItemPrice, SelectedCount * ItemPrice);
}

void UShopItemInfoWidget::LoadItemFromDataTable(int32 ItemID)
{
	if (!ItemDataTable)
	{
		LOG_Frame_WARNING(TEXT("ItemDataTable is null"));
		return;
	}

	const FString ContextString(TEXT("ShopItem Lookup"));
	TArray<FItemDataRow*> AllItems;
	ItemDataTable->GetAllRows(ContextString, AllItems);

	for (const FItemDataRow* ItemRow : AllItems)
	{
		if (ItemRow && ItemRow->ItemID == ItemID)
		{
			SetItemInfo(ItemRow->ItemIcon, ItemRow->ItemName, ItemRow->ItemDescription);
			ItemPrice = ItemRow->ItemPrice;
			return;
		}
	}

	LOG_Frame_WARNING(TEXT("Item with ID %d not found in ItemDataTable"), ItemID);
}

void UShopItemInfoWidget::SetItemInfo(UTexture2D* ItemIconImage, const FName& ItemNameText, const FText& ItemDescriptionText)
{
	if (ItemIcon && ItemIconImage)
	{
		ItemIcon->SetBrushFromTexture(ItemIconImage);
	}
	if (ItemName)
	{
		ItemName->SetText(FText::FromName(ItemNameText));
	}

	if (ItemDescription)
	{
		ItemDescription->SetText(ItemDescriptionText);
	}
}

