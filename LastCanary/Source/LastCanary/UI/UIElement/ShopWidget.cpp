#include "UI/UIElement/ShopWidget.h"
#include "UI/UIObject/ShopItemEntry.h"
#include "UI/UIObject/ShopItemInfoWidget.h"
#include "UI/UIObject/ShoppingCartWidget.h"
#include "UI/Manager/LCUIManager.h"

#include "Components/ScrollBox.h"
#include "Components/Button.h"

#include "Actor/LCDroneDelivery.h"
#include "Actor/LCDronePath.h"

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
		ExitButton->OnClicked.AddUniqueDynamic(this, &UShopWidget::OnExitButtonClicked);
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
		ExitButton->OnClicked.RemoveDynamic(this, &UShopWidget::OnExitButtonClicked);
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
	if (!ShoppingCartWidget || !DroneDeliveryClass)
	{
		LOG_Frame_WARNING(TEXT("ShopWidget missing references."));
		return;
	}

	// 1. 아이템 리스트 가져오기
	TArray<FItemDropData> DropList = ShoppingCartWidget->GetItemDropList();

	if (DropList.IsEmpty())
	{
		LOG_Frame_WARNING(TEXT("Cart is empty."));
		return;
	}

	// 2. 드론 액터 스폰
	FActorSpawnParameters Params;
	Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
	Params.Owner = GetOwningPlayerPawn();

	FVector SpawnLoc = GetOwningPlayerPawn()->GetActorLocation(); // 임시: 플레이어 위치에서 드론 스폰
	ALCDroneDelivery* Drone = GetWorld()->SpawnActor<ALCDroneDelivery>(DroneDeliveryClass, SpawnLoc, FRotator::ZeroRotator, Params);

	if (Drone)
	{
		Drone->ItemsToDrop = DropList;
		Drone->StartDelivery(); // 경로는 내부에서 자동 탐색
		LOG_Frame_WARNING(TEXT("Drone delivery started with %d items."), DropList.Num());
	}
	else
	{
		LOG_Frame_WARNING(TEXT("Drone delivery spawn failed."));
	}
}

void UShopWidget::OnExitButtonClicked()
{
	LOG_Frame_WARNING(TEXT("OnExitButtonClicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	if (UIManager)
	{
		UIManager->HideShopPopup();
	}
}
