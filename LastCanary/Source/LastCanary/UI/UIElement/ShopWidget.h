#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/DesktopWindowBaseWidget.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/ItemDropData.h"
#include "ShopWidget.generated.h"

class UScrollBox;
class UShopItemEntry;
class UButton;
class UDataTable;
class UShopItemInfoWidget;
class UShoppingCartWidget;

UCLASS()
class LASTCANARY_API UShopWidget : public UDesktopWindowBaseWidget
{
	GENERATED_BODY()

public:
	void SetGold(int gold);
	void OpenShopWidget();

	UShoppingCartWidget* GetShoppingCartWidget() const { return ShoppingCartWidget; }

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	virtual void ToggleMaximizeRestore() override;

	UFUNCTION()
	void OnShopItemClicked(UShopItemEntry* ClickedEntry);

	UFUNCTION()
	void OnPurchaseButtonClicked();

	void PopulateShopItems();

private:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemListBox;

	UPROPERTY(meta = (BindWidget))
	UButton* PurchaseButton;

	UPROPERTY(meta = (BindWidget))
	UShopItemInfoWidget* ItemInfoWidget;

	UPROPERTY(meta = (BindWidget))
	UShoppingCartWidget* ShoppingCartWidget;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	UDataTable* ItemDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopItemEntry> ShopItemEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	UTexture2D* ShopIconTexture;

	UShopItemEntry* CurrentlySelectedEntry = nullptr;
};
