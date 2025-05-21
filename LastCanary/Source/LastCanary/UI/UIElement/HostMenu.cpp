// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UIElement/HostMenu.h"
#include "Framework/GameMode/LCLobbyGameMode.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"


void UHostMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UHostMenu::OnStartButtonClicked);
	}
}

void UHostMenu::NativeDestruct()
{
	Super::NativeDestruct();

	if (StartButton)
	{
		StartButton->OnClicked.AddUniqueDynamic(this, &UHostMenu::OnStartButtonClicked);
	}
}

void UHostMenu::OnStartButtonClicked()
{
	if (ALCLobbyGameMode* LobbyGM = Cast<ALCLobbyGameMode>(UGameplayStatics::GetGameMode(this)))
	{
		LobbyGM->StartGame();
	}
}
