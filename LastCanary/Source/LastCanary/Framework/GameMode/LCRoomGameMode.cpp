#include "Framework/GameMode/LCRoomGameMode.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

#include "Actor/PlayerChecker.h"


ALCRoomGameMode::ALCRoomGameMode()
{

}

void ALCRoomGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	LOG_Server(Log, TEXT("1. InitGame 호출됨"));
	Super::InitGame(MapName, Options, ErrorMessage);

}

void ALCRoomGameMode::InitGameState()
{
	Super::InitGameState();

	LOG_Server(Log, TEXT("2. InitGameState 호출됨"));
}

void ALCRoomGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	LOG_Server(Log, TEXT("5. PostSeamlessTravel 호출됨"));

	// 레벨에 존재하는 PlayerChecker 재초기화
	for (TActorIterator<APlayerChecker> It(GetWorld()); It; ++It)
	{
		It->InitializeChecker();
		It->Server_OpenDoors();
	}
}

void ALCRoomGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	LOG_Server(Log, TEXT("4. PlayerController %s 가 심리스 트래블로 이관됨"), *C->GetName());

}

void ALCRoomGameMode::StartPlay()
{
	Super::StartPlay();

}

void ALCRoomGameMode::BeginPlay()
{
	Super::BeginPlay();
	LOG_Server(Log, TEXT("ALCRoomGameMode BeginPlay"));


}

void ALCRoomGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

}

//void ALCRoomGameMode::StartGame()
//{	
//
//}

void ALCRoomGameMode::UpdatePlayers()
{
	Super::UpdatePlayers();

	HandleBaseCampDoors();

}

void ALCRoomGameMode::HandleBaseCampDoors()
{
	bool bAllPlayersReady = IsAllPlayersReady();

	for (TActorIterator<APlayerChecker> It(GetWorld()); It; ++It)
	{
		if (bAllPlayersReady)
		{
			It->Server_CloseDoors();
		}
		else
		{
			It->Server_OpenDoors();
		}
	}
}
