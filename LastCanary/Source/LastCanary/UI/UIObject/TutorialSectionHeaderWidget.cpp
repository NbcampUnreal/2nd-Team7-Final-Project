#include "UI/UIObject/TutorialSectionHeaderWidget.h"
#include "Components/TextBlock.h"

void UTutorialSectionHeaderWidget::SetTitle(const FText& InTitle)
{
	if (TitleText)
	{
		TitleText->SetText(InTitle);
	}
}
