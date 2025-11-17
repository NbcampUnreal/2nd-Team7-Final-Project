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
class USizeBox;

/**
 * 상점 UI 위젯
 */
UCLASS()
class LASTCANARY_API UShopWidget : public UDesktopWindowBaseWidget
{
	GENERATED_BODY()

	//-----------------
	// 상점 열기 / 닫기
	//-----------------
public:
	void SetGold(int gold);
	void OpenShopWidget();

	virtual void OnCloseClicked() override;
	virtual void ToggleMaximizeRestore() override;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	//-----------------
	// 상점 아이템 관련
	//-----------------
protected:
	void PopulateShopItems();

	UFUNCTION()
	void OnShopItemClicked(UShopItemEntry* ClickedEntry);

	UFUNCTION()
	void OnPurchaseButtonClicked();

	//-----------------
	// 장바구니
	//-----------------
public:
	UShoppingCartWidget* GetShoppingCartWidget() const;

	//-----------------
	// 바인딩된 위젯
	//-----------------
private:
	UPROPERTY(meta = (BindWidget))
	USizeBox* RootSizeBox;

	UPROPERTY(meta = (BindWidget))
	UScrollBox* ItemListBox;

	UPROPERTY(meta = (BindWidget))
	UButton* PurchaseButton;

	UPROPERTY(meta = (BindWidget))
	UShopItemInfoWidget* ItemInfoWidget;

	UPROPERTY(meta = (BindWidget))
	UShoppingCartWidget* ShoppingCartWidget;

	//-----------------
	// 데이터 에셋
	//-----------------
private:
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	UDataTable* ItemDataTable;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<UShopItemEntry> ShopItemEntryClass;

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	UTexture2D* ShopIconTexture;

	//-----------------
	// 내부 상태
	//-----------------
private:
	UShopItemEntry* CurrentlySelectedEntry = nullptr;
};
