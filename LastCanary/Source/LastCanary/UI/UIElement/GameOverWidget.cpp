#include "UI/UIElement/GameOverWidget.h"

#include "Components/TextBlock.h"
#include "Components/Button.h"

#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "Framework/GameInstance/LCGameInstance.h"

#include "LastCanary.h"

void UGameOverWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (GameOverText)
	{
		GameOverText->SetText(FText::FromString(TEXT("Mission Failure\nNo Survivors Detected")));
	}

	if (GameOverButton)
	{
		GameOverButton->OnClicked.AddUniqueDynamic(this, &UGameOverWidget::OnGameOverButtonClicked);
	}
}

void UGameOverWidget::NativeDestruct()
{
	Super::NativeDestruct();

	if (GameOverButton)
	{
		GameOverButton->OnClicked.RemoveDynamic(this, &UGameOverWidget::OnGameOverButtonClicked);
	}
}

void UGameOverWidget::OnGameOverButtonClicked()
{
	if (APlayerController* PC = GetOwningPlayer())
	{
		PC->ClientReturnToMainMenuWithTextReason_Implementation(FText::FromString(TEXT("GameOver")));
	}
}

