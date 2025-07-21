#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/ShopItemEntry.h"
#include "UI/UIObject/ShopItemInfoWidget.h"
#include "UI/UIObject/ShoppingCartWidget.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Border.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Character/BasePlayerState.h"

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
	InitDesktopWindow();
}

void UShopWidget::NativeDestruct()
{
	Super::NativeDestruct();
}

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
}

void UShopWidget::OnShopItemClicked(UShopItemEntry* ClickedEntry)
{
	if (!ClickedEntry) return;

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
	if (ALCRoomPlayerController* PC = Cast<ALCRoomPlayerController>(GetOwningPlayer()))
	{
		PC->Server_RequestPurchase(ShoppingCartWidget->GetItemDropList());
	}

	ShoppingCartWidget->ClearCart();
	CloseWindow(); // 부모 기능 사용
}

void UShopWidget::PopulateShopItems()
{
	if (!ItemListBox || !ItemDataTable || !ShopItemEntryClass) return;

	ItemListBox->ClearChildren();

	TArray<FName> RowNames = ItemDataTable->GetRowNames();
	for (const FName& RowName : RowNames)
	{
		if (const FItemDataRow* ItemData = ItemDataTable->FindRow<FItemDataRow>(RowName, TEXT("Shop Load")))
		{
			if (!ItemData->bCanBuy)
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

FReply UShopWidget::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (InMouseEvent.GetEffectingButton() == EKeys::LeftMouseButton && IsInTitleBar(InMouseEvent.GetScreenSpacePosition()))
	{
		StartDragging(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return Super::NativeOnMouseButtonDown(InGeometry, InMouseEvent);
}

FReply UShopWidget::NativeOnMouseMove(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (bDragging && InMouseEvent.IsMouseButtonDown(EKeys::LeftMouseButton))
	{
		UpdateDrag(InMouseEvent.GetScreenSpacePosition());
		return FReply::Handled();
	}
	return Super::NativeOnMouseMove(InGeometry, InMouseEvent);
}

FReply UShopWidget::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	StopDragging();
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

bool UShopWidget::IsInTitleBar(const FVector2D& ScreenPos) const
{
	if (!TitleBar) return false;
	return TitleBar->GetCachedGeometry().IsUnderLocation(ScreenPos);
}

void UShopWidget::ToggleMaximizeRestore()
{
	Super::ToggleMaximizeRestore();
}
