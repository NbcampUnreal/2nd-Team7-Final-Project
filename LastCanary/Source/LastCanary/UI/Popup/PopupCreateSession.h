#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "DataType/SessionInfo.h"
#include "PopupCreateSession.generated.h"

class UButton;
class UComboBoxString;
class UEditableTextBox;

UCLASS()
class LASTCANARY_API UPopupCreateSession : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputServerName;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputPassword;

	UPROPERTY(meta = (BindWidget))
	UButton* PublicButton;
	UPROPERTY(meta = (BindWidget))
	UButton* PrivateButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateButton;
	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(BlueprintReadWrite)
	FSessionInfo CreateSessionInfo;

	UFUNCTION()
	void OnInputServerNameTextChanged(const FText& Text);
	UFUNCTION()
	void OnInputPasswordTextChanged(const FText& Text);

	void ToggleButtonClicked(bool bIsOn);
	UFUNCTION(BlueprintCallable)
	void OnPublicButtonClicked();
	UFUNCTION(BlueprintCallable)
	void OnPrivateButtonClicked();

	UFUNCTION()
	void OnCreateButtonClicked();
	UFUNCTION(BlueprintCallable)
	void OnExitButtonClicked();

private:
	void InitCreateSessionInfo();

	bool bIsPublic = false;
	int PlayerCount = 4;

	//UPROPERTY(meta = (BindWidget))
	//UComboBoxString* SlotComboBox;
	//UPROPERTY(BlueprintReadWrite)
	//int AmountOfSlots = 4;
	//UFUNCTION()
	//void OnSlotComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
