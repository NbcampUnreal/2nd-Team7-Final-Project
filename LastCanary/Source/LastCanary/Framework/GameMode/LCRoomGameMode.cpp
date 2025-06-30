#include "Framework/GameMode/LCRoomGameMode.h"

#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameState/LCGameState.h"

#include "Actor/LCGateActor.h"
#include "Character/BaseCharacter.h"
#include "Framework/Manager/GateCutsceneManager.h"
#include "Actor/PlayerChecker.h"

#include "EngineUtils.h"
#include "Kismet/GameplayStatics.h"

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

	if (ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>())
	{
		if (LCGM->CurrentRound == 0)
		{
			return;
		}
		if (LCGM->CurrentPlayerCount == CurrentPlayerNum)
		{
			OnAllPlayersJoined();
		}
	}
}

void ALCRoomGameMode::OnAllPlayersJoined()
{
	TArray<AActor*> FoundCharacters;

	UGameplayStatics::GetAllActorsOfClass(this, ABaseCharacter::StaticClass(), FoundCharacters);

	TArray<ABaseCharacter*> PlayerCharacters;
	for (AActor* Actor : FoundCharacters)
	{
		if (ABaseCharacter* BaseChar = Cast<ABaseCharacter>(Actor))
		{
			if (APlayerController* PC = Cast<APlayerController>(BaseChar->GetController()))
			{
				// Valid한 플레이어 컨트롤러가 소유한 캐릭터만 추가
				PlayerCharacters.Add(BaseChar);
			}
		}
	}

	ALCGateActor* GateActor = nullptr;
	for (TActorIterator<ALCGateActor> It(GetWorld()); It; ++It)
	{
		GateActor = *It;
		break;
	}

	// GateCutsceneManager 찾고 컷신 시작
	for (TActorIterator<AGateCutsceneManager> It(GetWorld()); It; ++It)
	{
		AGateCutsceneManager* GateCutsceneManager = *It;
		if (GateActor != nullptr)
		{
			GateCutsceneManager->LinkedGateActor = GateActor;
		}
		//서버에서 실행, 서버의 모든 플레이어 캐릭터의 정보를 게이트 매니저로 전송
		GateCutsceneManager->PlayGateCutscene(PlayerCharacters, ECutsceneType::GateExit);
		break;
	}
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
