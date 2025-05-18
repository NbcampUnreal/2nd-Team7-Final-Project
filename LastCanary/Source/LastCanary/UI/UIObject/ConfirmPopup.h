#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "ConfirmPopup.generated.h"

/**
 * 
 */
class UButton;
class UTextBlock;
UCLASS()
class LASTCANARY_API UConfirmPopup : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
public:
	void Init(TFunction<void()> OnConfirm);

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* YesButton;
	UPROPERTY(meta = (BindWidget))
	UButton* NoButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* MessageText;

private:
	TFunction<void()> ConfirmCallback;

	UFUNCTION()
	void OnYesClicked();
	UFUNCTION()
	void OnNoClicked();
};
