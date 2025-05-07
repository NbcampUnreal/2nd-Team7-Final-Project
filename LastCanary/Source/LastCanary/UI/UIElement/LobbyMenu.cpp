#include "UI/UIElement/LobbyMenu.h"
#include "Components/Button.h"
#include "Components/ScrollBox.h"

void ULobbyMenu::NativeConstruct()
{
	Super::NativeConstruct();
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.AddUniqueDynamic(this, &ULobbyMenu::OnCreateRoomButtonClicked);
	}
}

void ULobbyMenu::NativeDestruct()
{
	Super::NativeDestruct();
	if (CreateRoomButton)
	{
		CreateRoomButton->OnClicked.RemoveDynamic(this, &ULobbyMenu::OnCreateRoomButtonClicked);
	}
}

void ULobbyMenu::OnCreateRoomButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("Create Room Button Clicked"));
}
