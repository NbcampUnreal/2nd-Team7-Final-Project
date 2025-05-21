#include "Framework/GameMode/LCLobbyGameMode.h"
#include "Framework/PlayerController/LCLobbyPlayerController.h"
#include "DataType/RoomInfo.h"

void ALCLobbyGameMode::TryJoinRoom(ALCLobbyPlayerController* RequestingPC, const FString& RoomID, const FString& Password)
{
	// TODO : Implement the logic to join a room
	UE_LOG(LogTemp, Warning, TEXT("TryJoinRoom called with RoomID: %s, Password: %s"), *RoomID, *Password);
}

void ALCLobbyGameMode::StartGame()
{
    UWorld* World = GetWorld();
    if (World)
    {
        FString MapName = TEXT("/Game/_LastCanary/Dev/Server/MainLevel"); // ���� MainLevel ��η� �ٲټ���
        FString TravelURL = MapName + TEXT("?listen");

        World->ServerTravel(TravelURL, true);
    }
}

