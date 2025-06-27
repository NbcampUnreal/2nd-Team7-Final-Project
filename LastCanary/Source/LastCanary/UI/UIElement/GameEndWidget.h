#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "GameEndWidget.generated.h"

class UButton;
class UTextBlock;

UCLASS()
class LASTCANARY_API UGameEndWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* GameEndButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ContinueButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FinalResultMessage;

private:

	UFUNCTION()
	void OnGameEndButtonClicked();
	UFUNCTION(BlueprintCallable)
	void OnContinueButtonClicked();
};
