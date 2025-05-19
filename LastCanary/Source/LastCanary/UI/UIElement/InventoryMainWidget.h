#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "InventoryMainWidget.generated.h"

class UToolbarInventoryWidget;
class UBackpackInventoryWidget;

UCLASS()
class LASTCANARY_API UInventoryMainWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	

	UPROPERTY(meta = (BindWidget))
	UToolbarInventoryWidget* ToolbarWidget;

	UPROPERTY(meta = (BindWidget))
	UBackpackInventoryWidget* BackpackWidget;

public:
	void InitializeInventory(UToolbarInventoryWidget* InToolbarWidget, UBackpackInventoryWidget* InBackpackWidget);
	void RefreshInventory();
	void InitializeToolbarInventory(UToolbarInventoryWidget* InToolbarWidget);
	void InitializeBackpackInventory(UBackpackInventoryWidget* InBackpackWidget);
};
