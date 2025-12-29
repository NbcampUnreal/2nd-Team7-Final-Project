#include "UI/UIElement/PauseMenu.h"
#include "Components/Button.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Character/BasePlayerState.h"

#include "LastCanary.h"

FReply UPauseMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (InKeyEvent.GetKey() == EKeys::Escape)
	{
		OnResumeButtonClicked();
		return FReply::Handled();
	}
	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

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
	FSlateApplication::Get().SetKeyboardFocus(this->TakeWidget());
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
	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		ELCUIContext CurrentContext = UIManager->GetUIContext();
		UIManager->HidePauseMenu();
		UIManager->ChangeHUD();
		//switch (CurrentContext)
		//{
		//case ELCUIContext::Title:
		//	UIManager->ShowTitleMenu();
		//	break;
		//case ELCUIContext::Room:
		//	// falls through
		//case ELCUIContext::InGame:
		//{
		//	//APlayerController* PC = GetOwningPlayer();
		//	//if (!PC)
		//	//{
		//	//	return;
		//	//}
		//	//ABasePlayerState* MyPS = Cast<ABasePlayerState>(PC->PlayerState);
		//	//if (!MyPS)
		//	//{
		//	//	return;
		//	//}

		//	//if (MyPS->GetInGameStatus() == EPlayerInGameStatus::Spectating)
		//	//{
		//	//	UIManager->ShowHUD();
		//	//}
		//	//else
		//	//{
		//	//	UIManager->ShowHUD();
		//	//}
		//	//break;

		//}
		//default:
		//	UIManager->ShowHUD();
		//	break;
		//}
	}
}

void UPauseMenu::OnOptionButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Option Button Clicked"));
	ULCUIManager* LCUIManager = ResolveUIManager();
	if (LCUIManager)
	{
		LCUIManager->ShowOptionWidget();
	}
}

void UPauseMenu::OnExitButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Exit Button Clicked"));
	ULCUIManager* LCUIManager = ResolveUIManager();
	if (LCUIManager)
	{
		// 확인 팝업을 띄우고, 예를 눌렀을 경우에만 타이틀 메뉴로 전환
		LCUIManager->ShowConfirmPopup(
			[this]()
			{
				DestroySessionAndGoTitleMenu();
			},
			FText::FromString(TEXT("Are you sure you want to quit\r\n and return to the title menu?"))
		);
	}
}

void UPauseMenu::DestroySessionAndGoTitleMenu_Implementation()
{

}