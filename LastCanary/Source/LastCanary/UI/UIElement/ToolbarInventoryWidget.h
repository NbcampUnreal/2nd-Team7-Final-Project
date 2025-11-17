#pragma once

#include "CoreMinimal.h"
#include "UI/UIObject/InventoryWidgetBase.h"
#include "ToolbarInventoryWidget.generated.h"

class UHorizontalBox;

UCLASS()
class LASTCANARY_API UToolbarInventoryWidget : public UInventoryWidgetBase
{
	GENERATED_BODY()
	
public:
	UPROPERTY(meta = (BindWidget))
	UHorizontalBox* ToolbarSlotBox;

	UFUNCTION(BlueprintCallable)
	void RefreshInventoryUI() override;
};
