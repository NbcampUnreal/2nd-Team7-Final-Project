#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "ShopWidget.generated.h"

/**
 * 
 */
class UScrollBox;
class UShopItemEntry;
class UButton;
class UDataTable;
class UShopItemInfoWidget;
class UShoppingCartWidget;
UCLASS()
class LASTCANARY_API UShopWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemListBox;
	UPROPERTY(meta = (BindWidget))
	UButton* PurchaseButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UPROPERTY(meta = (BindWidget))
	UShopItemInfoWidget* ItemInfoWidget;
	UPROPERTY(meta = (BindWidget))
	UShoppingCartWidget* ShoppingCartWidget;

	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShopFadeAnim;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	UDataTable* ItemDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopItemEntry> ShopItemEntryClass;

	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void HandleItemClicked(const FItemDataRow& ClickedItem);
	UFUNCTION()
	void OnPurchaseButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();

	void PopulateShopItems();

public:
	UShoppingCartWidget* GetShoppingCartWidget() const { return ShoppingCartWidget; }

};
