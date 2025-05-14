#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "ShoppingCartWidget.generated.h"

/**
 * 
 */
class UScrollBox;
class UShoppingCartEntry;
UCLASS()
class LASTCANARY_API UShoppingCartWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	void AddItemToCart(const FItemDataRow& ItemData, int32 Quantity);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* CartItemBox;
	UPROPERTY(EditAnywhere, Category = "Cart")
	TSubclassOf<UShoppingCartEntry> ShoppingCartEntryClass;

private:
	UPROPERTY()
	TMap<int32, UShoppingCartEntry*> CartEntries;
};