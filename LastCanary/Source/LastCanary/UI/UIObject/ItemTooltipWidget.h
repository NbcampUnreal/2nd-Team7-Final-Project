#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataTable/ItemDataRow.h"
#include "DataType/InventoryItemData.h"
#include "ItemTooltipWidget.generated.h"

UCLASS()
class LASTCANARY_API UItemTooltipWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* NameText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* DescriptionText;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* QuantityText;

    UPROPERTY(meta = (BindWidgetOptional))
    class UTextBlock* DurabilityText;

    UFUNCTION(BlueprintCallable, Category = "Tooltip")
    virtual void SetTooltipData(const FItemDataRow& BaseData, const FInventoryItemData& InstanceData);
};
