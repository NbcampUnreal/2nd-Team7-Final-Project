#include "UI/UIElement/GameOverWidget.h"

#include "Components/TextBlock.h"

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
}

void UGameOverWidget::NativeDestruct()
{
	Super::NativeDestruct();
}
