#include "UI/UIObject/ItemTooltipWidget.h"
#include "Components/TextBlock.h"

void UItemTooltipWidget::SetTooltipData(const FItemDataRow& BaseData, const FBaseItemSlotData& InstanceData)
{
    if (NameText)
        NameText->SetText(FText::FromName(BaseData.ItemName));

    if (DescriptionText)
        DescriptionText->SetText(BaseData.ItemDescription);

    if (QuantityText)
        QuantityText->SetText(FText::AsNumber(InstanceData.Quantity));

    if (DurabilityText)
        DurabilityText->SetText(FText::Format(NSLOCTEXT("Tooltip", "Durability", "Durability: {0}"), FText::AsNumber(InstanceData.Durability)));
}
