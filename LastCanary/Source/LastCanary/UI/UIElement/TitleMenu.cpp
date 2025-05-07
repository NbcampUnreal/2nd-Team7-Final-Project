#include "LastCanary/UI/UIElement/TitleMenu.h"
#include "Components/Button.h"
#include "Kismet/KismetSystemLibrary.h"
#include "UI/Manager/LCUIManager.h"

void UTitleMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UTitleMenu::OnStartButtonClicked);
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
		UE_LOG(LogTemp, Warning, TEXT("KeyDown"));
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
	UE_LOG(LogTemp, Warning, TEXT("Start Button Clicked"));
}

void UTitleMenu::OnOptionButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Option Button Clicked"));
}

void UTitleMenu::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), nullptr, EQuitPreference::Quit, false);
}
