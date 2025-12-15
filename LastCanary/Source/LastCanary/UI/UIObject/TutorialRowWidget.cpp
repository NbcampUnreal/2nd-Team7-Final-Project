#include "UI/UIObject/TutorialRowWidget.h"
#include "Components/Image.h"
#include "Components/TextBlock.h"

void UTutorialRowWidget::Setup(UTexture2D* InIcon, const FText& InText)
{
	if (IconImage)
	{
		if (InIcon) IconImage->SetBrushFromTexture(InIcon, true);
		else        IconImage->SetBrushFromTexture(nullptr);
	}
	if (BodyText)
	{
		BodyText->SetAutoWrapText(true);
		BodyText->SetText(InText);
	}
}
