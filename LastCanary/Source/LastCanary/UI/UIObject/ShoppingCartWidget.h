#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "ShoppingCartWidget.generated.h"

/**
 * 
 */
class UScrollBox;
class UTextBlock;
class UShoppingCartEntry;
UCLASS()
class LASTCANARY_API UShoppingCartWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void AddItemToCart(const FItemDataRow& ItemData, int32 Quantity);
	void RecalculateTotalPrice();

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* CartItemBox;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalPriceText;

	UPROPERTY(EditAnywhere, Category = "Cart")
	TSubclassOf<UShoppingCartEntry> ShoppingCartEntryClass;

private:
	UPROPERTY()
	TMap<int32, UShoppingCartEntry*> CartEntries;
};