#include "LastCanary/UI/UIElement/TitleMenu.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

void UTitleMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UTitleMenu::OnStartButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.AddUniqueDynamic(this, &UTitleMenu::OnJoinButtonClicked);
	}
	if (OptionButton)
	{
		OptionButton->OnClicked.AddUniqueDynamic(this, &UTitleMenu::OnOptionButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.AddUniqueDynamic(this, &UTitleMenu::OnExitButtonClicked);
	}
}

void UTitleMenu::NativeDestruct()
{
	Super::NativeDestruct();
	if (StartButton)
	{
		StartButton->OnClicked.RemoveDynamic(this,&UTitleMenu::OnStartButtonClicked);
	}
	if (JoinButton)
	{
		JoinButton->OnClicked.RemoveDynamic(this, &UTitleMenu::OnJoinButtonClicked);
	}
	if (OptionButton)
	{
		OptionButton->OnClicked.RemoveDynamic(this, &UTitleMenu::OnOptionButtonClicked);
	}
	if (ExitButton)
	{
		ExitButton->OnClicked.RemoveDynamic(this, &UTitleMenu::OnExitButtonClicked);
	}
}

FReply UTitleMenu::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (!bHasActivated)
	{
		PlayAnimation(EnterAnimation);
		bHasActivated = true;
	}
	return FReply::Handled();
}

FReply UTitleMenu::NativeOnMouseButtonDown(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!bHasActivated)
	{
		PlayAnimation(EnterAnimation);
		bHasActivated = true;
	}
	return FReply::Handled();
}

void UTitleMenu::OnStartButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Start Button Clicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->ShowCreateSession();
	//ULCGameInstanceSubsystem* LCGameInstanceSubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	//int32 LevelID = GetTypeHash(FName(TEXT("LobbyMenu")));
	//LCGameInstanceSubsystem->ChangeLevelByMapID(LevelID);
}

void UTitleMenu::OnJoinButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Join Button Clicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->ShowLobbyMenu();
}

void UTitleMenu::OnOptionButtonClicked()
{
	LOG_Frame_WARNING(TEXT("Option Button Clicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->ShowOptionPopup();
}

void UTitleMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
