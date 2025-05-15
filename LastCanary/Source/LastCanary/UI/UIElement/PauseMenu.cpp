#include "UI/UIElement/PauseMenu.h"
#include "Components/Button.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

void UPauseMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (ResumeButton)
	{
		ResumeButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnResumeButtonClicked);
	}
	if (LobbyButton)
	{
		LobbyButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnLobbyButtonClicked);
	}
	if (OptionButton)
	{
		OptionButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnOptionButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UPauseMenu::OnExitButtonClicked);
	}
}

void UPauseMenu::NativeDestruct()
{
	Super::NativeDestruct();
	if (ResumeButton)
	{
		ResumeButton->OnClicked.RemoveDynamic(this, &UPauseMenu::OnResumeButtonClicked);
	}
	if (LobbyButton)
	{
		LobbyButton->OnClicked.RemoveDynamic(this, &UPauseMenu::OnLobbyButtonClicked);
	}
	if (OptionButton)
	{
		OptionButton->OnClicked.RemoveDynamic(this, &UPauseMenu::OnOptionButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UPauseMenu::OnExitButtonClicked);
	}
}

void UPauseMenu::OnResumeButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Resume Button Clicked"));
}

void UPauseMenu::OnLobbyButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Lobby Button Clicked"));
	if (ULCUIManager* LCUIManager = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>()->GetUIManager())
	{
		LCUIManager->ShowLobbyMenu();
	}
}

void UPauseMenu::OnOptionButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Option Button Clicked"));
	ULCUIManager* LCUIManager = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>()->GetUIManager();
	if (LCUIManager)
	{
		LCUIManager->ShowOptionPopup();
	}
}

void UPauseMenu::OnExitButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Exit Button Clicked"));
	ULCUIManager* LCUIManager = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>()->GetUIManager();
	if (LCUIManager)
	{
		LCUIManager->ShowTitleMenu();
	}
}