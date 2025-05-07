#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "TitleMenu.generated.h"

/**
 *
 */
class UButton;
UCLASS()
class LASTCANARY_API UTitleMenu : public ULCUserWidgetBase
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;
	virtual FReply NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;

private:
	bool bHasActivated = false;

	UPROPERTY(meta = (BindWidget))
	UButton* StartButton;
	UPROPERTY(meta = (BindWidget))
	UButton* OptionButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;
	UPROPERTY(meta = (BindWidgetAnim), Transient)
	UWidgetAnimation* EnterAnimation;

	UFUNCTION()
	void OnStartButtonClicked();
	UFUNCTION()
	void OnOptionButtonClicked();
	UFUNCTION()
	void OnExitButtonClicked();
};
