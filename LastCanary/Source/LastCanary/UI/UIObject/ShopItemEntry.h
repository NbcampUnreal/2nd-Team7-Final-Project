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

DECLARE_DELEGATE_OneParam(FOnShopItemClicked, UShopItemEntry*)

UCLASS()
class LASTCANARY_API UShopItemEntry : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

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
	void SetSelected(bool bInSelected);
	void InitItem(const FItemDataRow& InItemData);

	FOnShopItemClicked OnItemClicked;

	const FItemDataRow& GetItemData() const;
	
	int32 GetItemID() const;

private:
	UPROPERTY()
	bool bIsSelected = false;
	UPROPERTY()
	FItemDataRow ItemData;
	
	FButtonStyle DefaultButtonStyle;
};