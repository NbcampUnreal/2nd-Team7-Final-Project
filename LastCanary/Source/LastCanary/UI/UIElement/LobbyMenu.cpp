#include "UI/UIElement/LobbyMenu.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"
#include "Components/VerticalBox.h"
#include "UI/Manager/LCUIManager.h"

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

	if (ServerListBox)
	{
		ServerListBox->ClearChildren();
	}

	OnRefreshButtonClicked();

	StartAutoRefresh();
}

void ULobbyMenu::NativeDestruct()
{
	StopAutoRefresh();

	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.RemoveDynamic(this, &ULobbyMenu::OnCreateRoomButtonClicked);
	}
	if (RefreshButton)
	{
		RefreshButton->OnClicked.RemoveDynamic(this, &ULobbyMenu::OnRefreshButtonClicked);
	}
	if (BackButton)
	{
		BackButton->OnClicked.RemoveDynamic(this, &ULobbyMenu::OnBackButtonClicked);
	}

	Super::NativeDestruct();
}

void ULobbyMenu::OnCreateRoomButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Create Room Button Clicked"));
	ULCUIManager* UIManager = ResolveUIManager();
	UIManager->ShowCreateSession();
}

void ULobbyMenu::OnRefreshButtonClicked()
{
	if (ULCUIManager* UIManager = ResolveUIManager())
	{
		UIManager->ShowPopUpLoading();
	}

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

void ULobbyMenu::StartAutoRefresh()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().SetTimer(
			ServerListRefreshTimer,
			this,
			&ULobbyMenu::RefreshServerList,
			RefreshInterval,
			true
		);
	}
}

void ULobbyMenu::StopAutoRefresh()
{
	if (UWorld* W = GetWorld())
	{
		W->GetTimerManager().ClearTimer(ServerListRefreshTimer);
	}
}