#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "ShoppingCartEntry.generated.h"

/**
 * 
 */
class UImage;
class UTextBlock;
UCLASS()
class LASTCANARY_API UShoppingCartEntry : public ULCUserWidgetBase
{
	GENERATED_BODY()

public:
	void InitCartEntry(const FItemDataRow& InItemData, int32 InQuantity);
	void UpdateQuantity(int32 NewQuantity);
	int32 GetQuantity() const { return Quantity; }

	const FItemDataRow& GetItemData() const { return ItemData; }

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* QuantityText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalPriceText;

private:
	FItemDataRow ItemData;
	int32 Quantity = 1;
};