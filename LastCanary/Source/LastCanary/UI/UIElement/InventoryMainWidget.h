#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/BackpackSlotData.h"
#include "InventoryMainWidget.generated.h"

class UToolbarInventoryWidget;
class UBackpackInventoryWidget;
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

	FTimerHandle SlotItemTextTimerHandle;

private:
	bool bBackpackInventoryOpen = false;

	void AutoInitializeWithPlayer();

	UPROPERTY(EditAnywhere, Category = "WidgetClasses")
	TSubclassOf<UToolbarInventoryWidget> ToolbarWidgetClass;

	UPROPERTY(EditAnywhere, Category = "WidgetClasses")
	TSubclassOf<UBackpackInventoryWidget> BackpackWidgetClass;
	
public:
	void RefreshInventory();
};
