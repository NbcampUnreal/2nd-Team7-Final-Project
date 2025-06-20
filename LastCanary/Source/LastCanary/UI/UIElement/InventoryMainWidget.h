#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/BackpackSlotData.h"
#include "InventoryMainWidget.generated.h"

class UToolbarInventoryWidget;
class UBackpackInventoryWidget;
class UItemDropQuantityWidget;
class UInventorySlotWidget;
class UTextBlock;
struct FBaseItemSlotData;

UCLASS()
class LASTCANARY_API UInventoryMainWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta = (BindWidget))
	UToolbarInventoryWidget* ToolbarWidget;

	UPROPERTY(meta = (BindWidget))
	UBackpackInventoryWidget* BackpackWidget;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* SlotItemText;

public:
	virtual void NativeConstruct() override;

	virtual bool NativeOnDrop(const FGeometry& InGeometry, const FDragDropEvent& InDragDropEvent, UDragDropOperation* InOperation) override;

	UFUNCTION(BlueprintCallable)
	void ShowToolbarOnly();

	UFUNCTION(BlueprintCallable)
	void ToggleBackpackInventory();

	UFUNCTION(BlueprintCallable)
	bool IsBackpackInventoryOpen() const;

	UFUNCTION(BlueprintCallable)
	void ShowToolbarSlotItemText(const FText& ItemName);

	UFUNCTION()
	void HideToolbarSlotItemText();

	UFUNCTION(BlueprintCallable)
	void ShowItemDropQuantityWidget(UInventorySlotWidget* SourceWidget);

	UFUNCTION(BlueprintCallable)
	void HandleDropOutsideSlots(UInventorySlotWidget* SourceWidget, int32 Quantity);

	FTimerHandle SlotItemTextTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
	TSubclassOf<UItemDropQuantityWidget> ItemDropQuantityWidgetClass;

	UPROPERTY()
	UItemDropQuantityWidget* CurrentDropQuantityWidget;

private:
	bool bBackpackInventoryOpen = false;

	void AutoInitializeWithPlayer();

	UPROPERTY(EditAnywhere, Category = "WidgetClasses")
	TSubclassOf<UToolbarInventoryWidget> ToolbarWidgetClass;

	UPROPERTY(EditAnywhere, Category = "WidgetClasses")
	TSubclassOf<UBackpackInventoryWidget> BackpackWidgetClass;
	
	UFUNCTION()
	void OnQuantityConfirmed(int32 Quantity);

	UFUNCTION()
	void OnQuantityCanceled();

	UPROPERTY()
	UInventorySlotWidget* PendingDropSourceWidget;

public:
	void RefreshInventory();
};
