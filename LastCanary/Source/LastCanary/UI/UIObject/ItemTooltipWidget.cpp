#include "UI/UIObject/ItemTooltipWidget.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UItemTooltipWidget::SetTooltipData(const FItemDataRow& BaseData, const FBaseItemSlotData& InstanceData)
{
	if (IconImage)
	{
		IconImage->SetBrushFromTexture(BaseData.ItemIcon);
	}
	if (NameText)
	{
		NameText->SetText(FText::FromName(BaseData.ItemName));
	}
	if (QuantityText)
	{
		QuantityText->SetText(FText::AsNumber(InstanceData.Quantity));
	}
	if (DurabilityText)
	{
		if (InstanceData.Durability == 0.0f)
		{
			DurabilityText->SetText(FText::FromString(""));
		}
		else
		{
			DurabilityText->SetText(FText::Format(NSLOCTEXT("Tooltip", "Durability", "Durability: {0}"), FText::AsNumber(InstanceData.Durability)));
		}
	}
	if (DescriptionText)
	{
		DescriptionText->SetText(BaseData.ItemDescription);
	}
}