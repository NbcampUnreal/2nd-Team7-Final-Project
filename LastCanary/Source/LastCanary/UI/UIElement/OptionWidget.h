#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "OptionWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UGeneralOptionWidget;
UCLASS()
class LASTCANARY_API UOptionWidget : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

private:
	UPROPERTY(meta = (BindWidget)) 
	UWidgetSwitcher* OptionSwitcher;
	UPROPERTY(meta = (BindWidget)) 
	UButton* GeneralTabButton;
	UPROPERTY(meta = (BindWidget)) 
	UButton* KeySettingTabButton;
	UPROPERTY(meta = (BindWidget)) 
	UButton* ApplyButton;

	UPROPERTY(meta = (BindWidget))
	UGeneralOptionWidget* GeneralOptionWidget;
	

	UFUNCTION()
	void OnApplyButtonClicked();

	UFUNCTION()
	void OnGeneralTabButtonClicked();
};

