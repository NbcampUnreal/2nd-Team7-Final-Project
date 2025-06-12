#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ShopItemInfoWidget.generated.h"

class UImage;
class UTextBlock;
class UDataTable;
class UButton;
struct FItemDataRow;
class UShoppingCartWidget;
/**
 * 
 */
UCLASS()
class LASTCANARY_API UShopItemInfoWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	UShoppingCartWidget* ShoppingCartWidget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data")
	UDataTable* ItemDataTable;
	
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION(BlueprintCallable, Category = "Shop")
	int32 GetSelectedCount() const;
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetShoppingCartWidget(UShoppingCartWidget* InCartWidget);
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void LoadItemFromDataTable(int32 ItemID);
	UFUNCTION(BlueprintCallable, Category = "Shop")
	void SetItemInfo(UTexture2D* ItemIcon, const FName& ItemName, const FText& ItemDescription);

protected:
	UPROPERTY(meta = (BindWidget))
	UImage* ItemIcon;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemName;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* ItemDescription;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CountText;
	UPROPERTY(meta = (BindWidget))
	UButton* IncreaseButton;
	UPROPERTY(meta = (BindWidget))
	UButton* DecreaseButton;
	UPROPERTY(meta = (BindWidget))
	UButton* AddToCartButton;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = true))
	int32 ItemID = -1;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = true))
	int32 ItemPrice = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Data", meta = (AllowPrivateAccess = true))
	int32 SelectedCount = 1;
	UFUNCTION()
	void OnIncreaseButtonClicked();
	UFUNCTION()
	void OnDecreaseButtonClicked();
	UFUNCTION()
	void OnAddToCartButtonClicked();

	void UpdateCountDisplay();
};
