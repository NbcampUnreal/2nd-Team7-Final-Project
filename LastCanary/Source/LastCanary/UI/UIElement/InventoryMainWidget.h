#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "Item/ItemContainer.h"
#include "DataType/BackpackSlotData.h"
#include "DataTable/GunDataRow.h"
#include "InventoryMainWidget.generated.h"

class UToolbarInventoryWidget;
class UToolbarInventoryComponent;
class UBackpackInventoryWidget;
class UItemDropQuantityWidget;
class UInventorySlotWidget;
class UGunAmmoWidget;
class UItemContainerWidget;
class AGunBase;
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
	UItemContainerWidget* ContainerWidget;

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
	void HideItemDropQuantityWidget();

	UFUNCTION(BlueprintCallable)
	void HandleDropOutsideSlots(UInventorySlotWidget* SourceWidget, int32 Quantity);

	UFUNCTION(BlueprintCallable, Category = "Inventory|Drag")
	void CancelCurrentDragOperation();

	UFUNCTION(BlueprintPure, Category = "Inventory|Drag")
	bool IsDragInProgress() const;

	UFUNCTION(BlueprintCallable)
	void ShowContainerUI(AItemContainer* Container, const TArray<FContainerItemData>& ContainerItems);

	UFUNCTION(BlueprintCallable)
	void HideContainerUI();

	UFUNCTION(BlueprintCallable)
	bool IsContainerUIOpen() const;

	UToolbarInventoryWidget* GetToolbarWidget();
	UBackpackInventoryWidget* GetBackpackWidget();

	FTimerHandle SlotItemTextTimerHandle;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widget Classes")
	TSubclassOf<UItemDropQuantityWidget> ItemDropQuantityWidgetClass;

	UPROPERTY()
	UItemDropQuantityWidget* CurrentDropQuantityWidget;

	UPROPERTY()
	UItemContainerWidget* CurrentContainerWidget;

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

	bool bContainerUIOpen = false;

	UPROPERTY()
	AItemContainer* CurrentOpenContainer;

public:
	void RefreshInventory();

	UPROPERTY(meta = (BindWidget))
	UGunAmmoWidget* GunAmmoWidget;

	UFUNCTION(BlueprintCallable, Category = "UI|Gun")
	void SetGunAmmoUIVisibility();

private:
	void InitializeGunAmmoUI();

	void RestoreGunAmmoUIState();

	UToolbarInventoryComponent* GetOwnerToolbarComponent() const;
};
