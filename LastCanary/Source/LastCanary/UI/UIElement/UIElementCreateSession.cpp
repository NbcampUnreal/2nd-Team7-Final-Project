// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIElement/UIElementCreateSession.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"

void UUIElementCreateSession::NativeConstruct()
{
	Super::NativeConstruct();
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UUIElementCreateSession::OnExitButtonClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.AddUniqueDynamic(this, &UUIElementCreateSession::OnCreateButtonClicked);
	}
	if (InputServerName)
	{
		InputServerName->OnTextChanged.AddUniqueDynamic(this, &UUIElementCreateSession::OnInputServerNameTextChanged);
		ServerName = InputServerName->GetText().ToString();
	}
	if (SlotComboBox)
	{
		SlotComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UUIElementCreateSession::OnSlotComboBoxSelectionChanged);
		AmountOfSlots = FCString::Atoi(*SlotComboBox->GetSelectedOption());
	}
}

void UUIElementCreateSession::NativeDestruct()
{
	Super::NativeDestruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UUIElementCreateSession::OnExitButtonClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.AddUniqueDynamic(this, &UUIElementCreateSession::OnCreateButtonClicked);
	}
	if (InputServerName)
	{
		InputServerName->OnTextChanged.AddUniqueDynamic(this, &UUIElementCreateSession::OnInputServerNameTextChanged);
	}
	if (SlotComboBox)
	{
		SlotComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UUIElementCreateSession::OnSlotComboBoxSelectionChanged);
	}
}


void UUIElementCreateSession::OnExitButtonClicked()
{
	RemoveFromParent();
	Destruct();
}

void UUIElementCreateSession::OnCreateButtonClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULCGameInstance* LCGameInstance = Cast<ULCGameInstance>(GameInstance))
		{
			LCGameInstance->CreateSession(ServerName, AmountOfSlots);
		}
	}
}

void UUIElementCreateSession::OnInputServerNameTextChanged(const FText& Text)
{
	ServerName = Text.ToString();
}

void UUIElementCreateSession::OnSlotComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	AmountOfSlots = FCString::Atoi(*SelectedItem);
}
