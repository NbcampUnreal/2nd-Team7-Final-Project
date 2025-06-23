#include "UI/UIObject/PlayerNameWidget.h"
#include "Components/TextBlock.h"

void UPlayerNameWidget::SetPlayerName(const FString& InName)
{
	if (!PlayerNameText)
	{
		UE_LOG(LogTemp, Error, TEXT("[UPlayerNameWidget] PlayerNameText is NULL"));
		return;
	}

	UE_LOG(LogTemp, Warning, TEXT("[UPlayerNameWidget] SetPlayerName: %s"), *InName);
	PlayerNameText->SetText(FText::FromString(InName));
}
