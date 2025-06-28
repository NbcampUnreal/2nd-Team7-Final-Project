#include "UI/UIElement/GameEndWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"


void UGameEndWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GameEndButton)
	{
		GameEndButton->OnClicked.AddUniqueDynamic(this, &UGameEndWidget::OnGameEndButtonClicked);
	}

	if (ContinueButton)
	{
		ContinueButton->OnClicked.AddUniqueDynamic(this, &UGameEndWidget::OnContinueButtonClicked);
	}
}

void UGameEndWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (GameEndButton)
	{
		GameEndButton->OnClicked.RemoveDynamic(this, &UGameEndWidget::OnGameEndButtonClicked);
	}
	if (ContinueButton)
	{
		ContinueButton->OnClicked.RemoveDynamic(this, &UGameEndWidget::OnContinueButtonClicked);
	}
}

void UGameEndWidget::OnGameEndButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ClientReturnToMainMenuWithTextReason_Implementation(FText::FromString(TEXT("Game End")));
	}
}

void UGameEndWidget::OnContinueButtonClicked()
{

}
