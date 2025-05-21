#include "UI/UIElement/LobbyMenu.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"

void ULobbyMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::OnCreateRoomButtonClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::OnRefreshButtonClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::OnBackButtonClicked);
	}

	RefreshServerList();
}

void ULobbyMenu::NativeDestruct()
{
	Super::NativeDestruct();
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.RemoveDynamic(this, &ULobbyMenu::OnCreateRoomButtonClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.RemoveDynamic(this, &ULobbyMenu::OnRefreshButtonClicked);
	}
}

void ULobbyMenu::OnCreateRoomButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Create Room Button Clicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->ShowCreateSession();
}

void ULobbyMenu::OnRefreshButtonClicked()
{
	RefreshServerList();
}

void ULobbyMenu::RefreshServerList_Implementation()
{

}

void ULobbyMenu::OnBackButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Create Room Button Clicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->ShowTitleMenu();
}
