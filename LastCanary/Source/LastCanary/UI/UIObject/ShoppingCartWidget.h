#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/ItemDropData.h"
#include "ShoppingCartWidget.generated.h"

/**
 * 
 */
class UScrollBox;
class UTextBlock;
class UShoppingCartEntry;

DECLARE_DELEGATE_OneParam(FOnCartValidityChanged, bool); // 구매 가능 여부 델리게이트

UCLASS()
class LASTCANARY_API UShoppingCartWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void AddItemToCart(const FItemDataRow& ItemData, int32 Quantity);
	
	UFUNCTION()
	void HandleRemoveCartEntry(UShoppingCartEntry* EntryToRemove);
	
	void RecalculateTotalPrice();
	TArray<FItemDropData> GetItemDropList() const;
	void ClearCart();

	/** 외부에서 골드 값을 넘겨줌 */
	void SetPlayerGold(int32 NewGold);

	/** 델리게이트로 유효성 알림 */
	FOnCartValidityChanged OnCartValidityChanged;

protected:
	UPROPERTY(meta = (BindWidget))
	UScrollBox* CartItemBox;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* TotalPriceText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* PlayerGoldText;

	UPROPERTY(EditAnywhere, Category = "Cart")
	TSubclassOf<UShoppingCartEntry> ShoppingCartEntryClass;

private:
	UPROPERTY()
	TMap<int32, UShoppingCartEntry*> CartEntries;

	int32 TotalPrice = 0;
	int32 PlayerGold = 0;

	void UpdateVisuals();
};