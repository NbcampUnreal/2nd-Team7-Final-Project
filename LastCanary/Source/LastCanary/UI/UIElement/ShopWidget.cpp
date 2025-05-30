#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/ShopItemEntry.h"
#include "UI/UIObject/ShopItemInfoWidget.h"
#include "UI/UIObject/ShoppingCartWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"

#include "Actor/LCDroneDelivery.h"
#include "Actor/LCDronePath.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"

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
		ExitButton->OnClicked.AddUniqueDynamic(this, &UShopWidget::CloseShopWidget);
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
		ExitButton->OnClicked.RemoveDynamic(this, &UShopWidget::CloseShopWidget);
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
				ItemEntry->OnItemClicked.BindUObject(this, &UShopWidget::OnShopItemClicked);
				ItemListBox->AddChild(ItemEntry);

				// LOG_Frame_WARNING(TEXT("Shop item bound: %s"), *ItemData->ItemName.ToString());
			}
		}
	}
}

void UShopWidget::OnShopItemClicked(UShopItemEntry* ClickedEntry)
{
	LOG_Frame_WARNING(TEXT("OnShopItemClicked"));
	if (ClickedEntry == nullptr)
	{
		return;
	}

	if (CurrentlySelectedEntry)
	{
		CurrentlySelectedEntry->SetSelected(false);
	}

	CurrentlySelectedEntry = ClickedEntry;
	ClickedEntry->SetSelected(true);

	if (ItemInfoWidget)
	{
		ItemInfoWidget->ItemDataTable = ItemDataTable;
		ItemInfoWidget->LoadItemFromDataTable(ClickedEntry->GetItemID());
	}
}

void UShopWidget::OnPurchaseButtonClicked()
{
	ALCRoomPlayerController* PC = Cast<ALCRoomPlayerController>(GetOwningPlayer());
	if (PC)
	{
		PC->Server_RequestPurchase(ShoppingCartWidget->GetItemDropList());
	}
	ShoppingCartWidget->ClearCart();
	CloseShopWidget();
}

void UShopWidget::CloseShopWidget()
{
	ULCUIManager* UIManager = ResolveUIManager();
	if (UIManager)
	{
		UIManager->HideShopPopup();
	}
}
