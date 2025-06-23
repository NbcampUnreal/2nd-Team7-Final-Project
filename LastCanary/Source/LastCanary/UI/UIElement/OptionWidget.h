#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "OptionWidget.generated.h"

class UWidgetSwitcher;
class UButton;
class UGeneralOptionWidget;
class UKeySettingWidget;
class UVoiceOptionWidget;
class UGraphicsSettingPanel;
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
	UButton* GraphicsOptionButton;
	UPROPERTY(meta = (BindWidget)) 
	UButton* KeySettingTabButton;
	UPROPERTY(meta = (BindWidget)) 
	UButton* VoiceOptionTabButton;

	UPROPERTY(meta = (BindWidget)) 
	UButton* ApplyButton;
	UPROPERTY(meta = (BindWidget)) 
	UButton* CloseButton;
	UPROPERTY(meta = (BindWidget)) 
	UButton* ResetButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* GeneralTabText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* GraphicsOptionTabText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* KeySettingTabText;
	UPROPERTY(meta = (BindWidget))
	UTextBlock* VoiceOptionTabText;

	UPROPERTY(meta = (BindWidget))
	UGeneralOptionWidget* GeneralOptionWidget;
	UPROPERTY(meta = (BindWidget))
	UGraphicsSettingPanel* GraphicsSettingPanel;
	UPROPERTY(meta = (BindWidget))
	UKeySettingWidget* KeySettingWidget;
	UPROPERTY(meta = (BindWidget))
	UVoiceOptionWidget* VoiceOptionWidget;
	
	UFUNCTION()
	void OnApplyButtonClicked();
	UFUNCTION()
	void OnCloseButtonClicked();
	UFUNCTION()
	void OnResetButtonClicked();

	UFUNCTION()
	void OnGeneralTabButtonClicked();
	UFUNCTION()
	void OnGraphicsSettingButtonClicked();
	UFUNCTION()
	void OnKeySettingTabButtonClicked();
	UFUNCTION()
	void OnVoiceOptionTabButtonClicked();

public:
	UKeySettingWidget* GetKeySettingWidget() const { return KeySettingWidget; }
};

