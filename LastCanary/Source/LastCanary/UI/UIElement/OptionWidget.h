#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "OptionWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UGeneralOptionWidget;
class UKeySettingWidget;
class UTextBlock;
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
	UButton* CloseButton;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GeneralTabText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KeySettingTabText;


	UPROPERTY(meta = (BindWidget))
	UGeneralOptionWidget* GeneralOptionWidget;
	UPROPERTY(meta = (BindWidget))
	UKeySettingWidget* KeySettingWidget;
	
	//FLinearColor SelectedColor = FLinearColor(0.87f, 0.87f, 0.87f);   
	//FLinearColor UnselectedColor = FLinearColor(0.5f, 0.5f, 0.5f);    

	UFUNCTION()
	void OnApplyButtonClicked();
	UFUNCTION()
	void OnCloseButtonClicked();
	UFUNCTION()
	void OnGeneralTabButtonClicked();
	UFUNCTION()
	void OnKeySettingTabButtonClicked();

public:
	/* 위젯 게터 */
	UKeySettingWidget* GetKeySettingWidget() const { return KeySettingWidget; }
	//void SetTabButtonStyle(UButton* Button, bool bIsSelected);
};

