#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/ShopItemEntry.h"

#include "UI/UIObject/ShopItemInfoWidget.h"
#include "UI/UIObject/ShoppingCartWidget.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/SizeBox.h"
#include "Components/CanvasPanelSlot.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Character/BasePlayerState.h"

#include "UI/Manager/LCDesktopWindowManager.h"

//-----------------
// 시스템 초기화
//-----------------
void UShopWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (PurchaseButton)
	{
		PurchaseButton->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnPurchaseButtonClicked);
		PurchaseButton->SetIsEnabled(false);
	}

	if (ItemInfoWidget && ShoppingCartWidget)
	{
		ItemInfoWidget->SetShoppingCartWidget(ShoppingCartWidget);
	}

	if (ShoppingCartWidget)
	{
		ShoppingCartWidget->OnCartValidityChanged.BindLambda([this](bool bCanAfford)
			{
				if (PurchaseButton)
				{
					PurchaseButton->SetIsEnabled(bCanAfford);
				}
			});
	}

	PopulateShopItems();
}

void UShopWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

//-----------------
// 외부 인터페이스
//-----------------
void UShopWidget::SetGold(int gold)
{
	if (ShoppingCartWidget)
	{
		ShoppingCartWidget->SetPlayerGold(gold);
	}
}

void UShopWidget::OpenShopWidget()
{
	SetVisibility(ESlateVisibility::Visible);

	SetMaximized(false);

	if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(Slot))
	{
		CanvasSlot->SetPosition(GetOriginalPosition());
		CanvasSlot->SetSize(GetOriginalSize());
	}
}

UShoppingCartWidget* UShopWidget::GetShoppingCartWidget() const
{
	return ShoppingCartWidget;
}

//-----------------
// 상점 아이템 처리
//-----------------
void UShopWidget::OnShopItemClicked(UShopItemEntry* ClickedEntry)
{
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

void UShopWidget::PopulateShopItems()
{
	if (ItemListBox == nullptr)
	{
		return;
	}
	if (ItemDataTable == nullptr)
	{
		return;
	}
	if (ShopItemEntryClass == nullptr)
	{
		return;
	}

	ItemListBox->ClearChildren();
	TArray<FName> RowNames = ItemDataTable->GetRowNames();

	for (const FName& RowName : RowNames)
	{
		if (const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("Shop Load")))
		{
			if (ItemData->bCanBuy == false)
			{
				continue;
			}

			UShopItemEntry* ItemEntry = CreateWidget<UShopItemEntry>(this, ShopItemEntryClass);
			if (ItemEntry)
			{
				ItemEntry->InitItem(*ItemData);
				ItemEntry->OnItemClicked.BindUObject(this, &UShopWidget::OnShopItemClicked);
				ItemListBox->AddChild(ItemEntry);
			}
		}
	}
}

//-----------------
// 버튼 콜백
//-----------------
void UShopWidget::OnPurchaseButtonClicked()
{
	if (ShoppingCartWidget == nullptr)
	{
		return;
	}

	if (ALCRoomPlayerController* PC = Cast<ALCRoomPlayerController>(GetOwningPlayer()))
	{
		PC->Server_RequestPurchase(ShoppingCartWidget->GetItemDropList());
	}

	ShoppingCartWidget->ClearCart();

	if (WindowManager)
	{
		WindowManager->CloseWindow(this);
	}
	else
	{
		RemoveFromParent();
	}
}

void UShopWidget::OnCloseClicked()
{
	if (ShoppingCartWidget)
	{
		ShoppingCartWidget->ClearCart(); 
	}

	Super::OnCloseClicked();
}

void UShopWidget::ToggleMaximizeRestore()
{
	Super::ToggleMaximizeRestore();
}
