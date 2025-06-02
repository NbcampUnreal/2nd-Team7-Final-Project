#include "UI/Popup/PopupCreateSession.h"

#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Components/Button.h"
#include "Components/EditableTextBox.h"
#include "Components/ComboBoxString.h"
#include "DataType/SessionInfo.h"

void UPopupCreateSession::NativeConstruct()
{
	Super::NativeConstruct();
	if (InputServerName)
	{
		InputServerName->OnTextChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnInputServerNameTextChanged);
		//CreateSessionInfo.ServerName = InputServerName->GetText().ToString();
	}
	if (InputPassword)
	{
		InputPassword->OnTextChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnInputPasswordTextChanged);
		//CreateSessionInfo.Password = InputPassword->GetText().ToString();
	}
	if (PublicButton)
	{
		PublicButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnPublicButtonClicked);
	}
	if (PrivateButton)
	{
		PrivateButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnPrivateButtonClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnCreateButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UPopupCreateSession::OnExitButtonClicked);
	}

	InitCreateSessionInfo();
	//if (SlotComboBox)
	//{
	//	SlotComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnSlotComboBoxSelectionChanged);
	//	AmountOfSlots = FCString::Atoi(*SlotComboBox->GetSelectedOption());
	//}
}

void UPopupCreateSession::NativeDestruct()
{
	Super::NativeDestruct();

	if (InputServerName)
	{
		InputServerName->OnTextChanged.RemoveDynamic(this, &UPopupCreateSession::OnInputServerNameTextChanged);
	}
	if (InputPassword)
	{
		InputPassword->OnTextChanged.RemoveDynamic(this, &UPopupCreateSession::OnInputPasswordTextChanged);
	}
	if (PublicButton)
	{
		PublicButton->OnClicked.RemoveDynamic(this, &UPopupCreateSession::OnPublicButtonClicked);
	}
	if (PrivateButton)
	{
		PrivateButton->OnClicked.RemoveDynamic(this, &UPopupCreateSession::OnPrivateButtonClicked);
	}
	if (CreateButton)
	{
		CreateButton->OnClicked.RemoveDynamic(this, &UPopupCreateSession::OnCreateButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UPopupCreateSession::OnExitButtonClicked);
	}
	//if (SlotComboBox)
	//{
	//	SlotComboBox->OnSelectionChanged.AddUniqueDynamic(this, &UPopupCreateSession::OnSlotComboBoxSelectionChanged);
	//}
}

void UPopupCreateSession::InitCreateSessionInfo()
{
	CreateSessionInfo.ServerName = InputServerName->GetText().ToString();
	CreateSessionInfo.bIsPublic = bIsPublic;
	CreateSessionInfo.Password = InputPassword->GetText().ToString();
	CreateSessionInfo.MaxPlayerNum = PlayerCount;

	ToggleButtonClicked(true);
}


void UPopupCreateSession::OnInputServerNameTextChanged(const FText& Text)
{
	CreateSessionInfo.ServerName = Text.ToString();
}

void UPopupCreateSession::OnInputPasswordTextChanged(const FText& Text)
{
	CreateSessionInfo.Password = Text.ToString();
}

void UPopupCreateSession::ToggleButtonClicked(bool bIsOn)
{
	bIsPublic = bIsOn;
	CreateSessionInfo.bIsPublic = bIsPublic;

	PublicButton->SetIsEnabled(!bIsOn);

	PrivateButton->SetIsEnabled(bIsOn);// = bIsOn;

	InputPassword->SetIsEnabled(!bIsOn);
	if (bIsOn)
	{
		InputPassword->SetText(FText::GetEmpty());
	}
}

void UPopupCreateSession::OnPublicButtonClicked()
{
	ToggleButtonClicked(true);
}

void UPopupCreateSession::OnPrivateButtonClicked()
{
	ToggleButtonClicked(false);
}

void UPopupCreateSession::OnCreateButtonClicked()
{
	if (UGameInstance* GameInstance = GetGameInstance())
	{
		if (ULCGameInstance* LCGameInstance = Cast<ULCGameInstance>(GameInstance))
		{
			LCGameInstance->CreateSession(CreateSessionInfo);
		}
	}
}

void UPopupCreateSession::OnExitButtonClicked()
{
	RemoveFromParent();
	Destruct();
}

//void UPopupCreateSession::OnSlotComboBoxSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
//{
//	AmountOfSlots = FCString::Atoi(*SelectedItem);
//}
