#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "ShopItemEntry.generated.h"
/**
 * 
 */
class UImage;
class UTextBlock;
class UButton;
UCLASS()
class LASTCANARY_API UShopItemEntry : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnSelectButtonClicked();

	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemNameText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemPriceText;
	UPROPERTY(meta = (BindWidget))
	UButton* SelectButton;
	
public:
	void InitItem(const FItemDataRow& InItemData);

private:
	FItemDataRow ItemData;
};