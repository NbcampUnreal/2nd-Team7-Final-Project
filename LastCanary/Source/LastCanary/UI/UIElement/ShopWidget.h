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
UCLASS()
class LASTCANARY_API UShopWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemListBox;
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	UDataTable* ItemDataTable;
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopItemEntry> ShopItemEntryClass;
	UPROPERTY(meta = (BindWidget))
	UButton* PurchaseButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* ShopFadeAnim;

	void PopulateShopItems();

	UFUNCTION()
	void HandleItemClicked(const FItemDataRow& ClickedItem);
	UFUNCTION()
	void OnPurchaseButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();

};
