// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UI/LCUserWidgetBase.h"
#include "UIElementCreateSession.generated.h"

/**
 * 
 */

class UButton;
class UComboBoxString;
class UEditableTextBox;

UCLASS()
class LASTCANARY_API UUIElementCreateSession : public ULCUserWidgetBase
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UPROPERTY(meta = (BindWidget))
	UButton* ExitButton;

	UPROPERTY(meta = (BindWidget))
	UButton* CreateButton;

	UPROPERTY(meta = (BindWidget))
	UEditableTextBox* InputServerName;

	UPROPERTY(meta = (BindWidget))
	UComboBoxString* SlotComboBox;



	UPROPERTY(BlueprintReadWrite)
	FString ServerName;

	UPROPERTY(BlueprintReadWrite)
	int AmountOfSlots;

	UFUNCTION(BlueprintCallable)
	void OnExitButtonClicked();
	UFUNCTION()
	void OnCreateButtonClicked();
	UFUNCTION()
	void OnInputServerNameTextChanged(const FText& Text);
	UFUNCTION()
	void OnSlotComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
