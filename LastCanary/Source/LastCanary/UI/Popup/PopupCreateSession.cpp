#include "UI/Popup/PopupCreateSession.h"

#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"

void UPopupCreateSession::NativeConstruct()
{
	Super::NativeConstruct();
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnExitButtonClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnCreateButtonClicked);
	}
	if (InputServerName)
	{
		InputServerName->OnTextChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnInputServerNameTextChanged);
		ServerName = InputServerName->GetText().ToString();
	}
	if (SlotComboBox)
	{
		SlotComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnSlotComboBoxSelectionChanged);
		AmountOfSlots = FCString::Atoi(*SlotComboBox->GetSelectedOption());
	}
}

void UPopupCreateSession::NativeDestruct()
{
	Super::NativeDestruct();

	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnExitButtonClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnCreateButtonClicked);
	}
	if (InputServerName)
	{
		InputServerName->OnTextChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnInputServerNameTextChanged);
	}
	if (SlotComboBox)
	{
		SlotComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnSlotComboBoxSelectionChanged);
	}
}


void UPopupCreateSession::OnExitButtonClicked()
{
	RemoveFromParent();
	Destruct();
}

void UPopupCreateSession::OnCreateButtonClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULCGameInstance* LCGameInstance = Cast<ULCGameInstance>(GameInstance))
		{
			LCGameInstance->CreateSession(ServerName, AmountOfSlots);
		}
	}
}

void UPopupCreateSession::OnInputServerNameTextChanged(const FText& Text)
{
	ServerName = Text.ToString();
}

void UPopupCreateSession::OnSlotComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	AmountOfSlots = FCString::Atoi(*SelectedItem);
}
