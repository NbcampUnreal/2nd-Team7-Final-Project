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
	RemoveFromParent();
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->SetInputModeGameOnly();
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
	// UI 매니저 획득
	ULCUIManager* LCUIManager = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>()->GetUIManager();
	if (LCUIManager)
	{
		// 확인 팝업을 띄우고, 예를 눌렀을 경우에만 타이틀 메뉴로 전환
		LCUIManager->ShowConfirmPopup(
			[]()
			{
				//TODO : 타이틀 메뉴로 전환
			}
		);
	}
}