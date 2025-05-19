#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "UI/UIObject/ItemTooltipWidget.h"
#include "DataType/BaseItemSlotData.h"
#include "Inventory/InventoryComponentBase.h"
#include "InventorySlotWidget.generated.h"

UCLASS()
class LASTCANARY_API UInventorySlotWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	FBaseItemSlotData ItemData;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	UDataTable* ItemDataTable;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetItemData(const FBaseItemSlotData& InItemSlotData, UDataTable* InItemDataTable);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void SetInventoryComponent(UInventoryComponentBase* InInventoryComponent);

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	virtual void UpdateSlotUI();

	// 해당 함수는 블루프린트로 효과를 추가하고 싶을 때 사용 현재는 사용 안함
	//UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	//void OnSlotHovered();

	//UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	//void OnSlotUnhovered();

	//UFUNCTION(BlueprintImplementableEvent, Category = "Inventory")
	//void OnSlotSelected();

	//UFUNCTION(BlueprintImplementableEvent, Category = "inventory")
	//void OnSlotDragStarted();

	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnMouseLeave(const FPointerEvent& InMouseEvent) override;
	virtual void NativeOnDragDetected(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent, UDragDropOperation*& OutOperation) override;
	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void ShowTooltip();

	UFUNCTION(BlueprintCallable, Category = "Inventory")
	void HideTooltip();

	UFUNCTION()
	void OnUseButtonClicked();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<UItemTooltipWidget> TooltipWidgetClass;

	UPROPERTY()
	UItemTooltipWidget* ItemTooltipWidget;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemNameText;

	UPROPERTY(meta = (BindWidget))
	class UImage* ItemIconImage;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* ItemQuantityText;

	UPROPERTY(BlueprintReadOnly, Category = "Inventory")
	int32 SlotIndex = -1;
	
	UPROPERTY(BlueprintReadOnly, Category = "inventory")
	UInventoryComponentBase* InventoryComponent;
};
