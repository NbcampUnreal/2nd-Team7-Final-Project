#include "Framework/GameMode/LCRoomGameMode.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"

#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"

#include "EngineUtils.h"
#include "Actor/PlayerChecker.h"

#include "Kismet/GameplayStatics.h"


ALCRoomGameMode::ALCRoomGameMode()
{

}

void ALCRoomGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	UE_LOG(LogTemp, Log, TEXT("1. InitGame 호출됨"));
	Super::InitGame(MapName, Options, ErrorMessage);

	//ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>();
	//if (LCGM)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("이전 맵에서 넘어온 플레이어 수: %d"), LCGM->CurrentPlayerCount);
	//	InGamePlayerNum = LCGM->CurrentPlayerCount;
	//}

	//ULCGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	//if (GI)
	//{
	//	//UE_LOG(LogTemp, Log, TEXT("이전 맵에서 넘어온 플레이어 수: %d"), GI->UserNum);
	//	//InGamePlayerNum = GI->UserNum;
	//}
}

void ALCRoomGameMode::InitGameState()
{
	Super::InitGameState();

	UE_LOG(LogTemp, Log, TEXT("2. InitGameState 호출됨"));
}

void ALCRoomGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	UE_LOG(LogTemp, Log, TEXT("5. PostSeamlessTravel 호출됨"));

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

	UE_LOG(LogTemp, Log, TEXT("4. PlayerController %s 가 심리스 트래블로 이관됨"), *C->GetName());

	//ConnectedPlayers++;
	//ALCGameState* LCGameState = GetGameState<ALCGameState>();
	//if (!IsValid(LCGameState))
	//{
	//	return;

	//}
	//if (InGamePlayerNum <= ConnectedPlayers)
	//{
	//	UE_LOG(LogTemp, Log, TEXT("모든 플레이어 연결 완료!"));
	//	//StartGame();
	//}
}

void ALCRoomGameMode::StartPlay()
{
	Super::StartPlay();

}

void ALCRoomGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ALCRoomGameMode BeginPlay"));

	// 생존자 수 초기화
	//if (ALCGameState* LCGameState = GetGameState<ALCGameState>())
	//{
	//	int32 InitialAliveCount = 0;

	//	for (APlayerState* PlayerState : LCGameState->PlayerArray)
	//	{
	//		if (IsValid(PlayerState) && !PlayerState->IsOnlyASpectator())
	//		{
	//			++InitialAliveCount;
	//		}
	//	}

	//	LCGameState->AlivePlayerCount = InitialAliveCount;
	//}
}

void ALCRoomGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	//for (TActorIterator<APlayerChecker> It(GetWorld()); It; ++It)
	//{
	//	It->Server_OpenDoors(); // 강제로 다시 열어줌 S
	//}

	//if (ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>())
	//{
	//	if (LCGM->CurrentPlayerCount == CurrentPlayerNum)
	//	{
	//		StartGame();
	//	}
	//}

}

void ALCRoomGameMode::StartGame()
{	
	//UE_LOG(LogTemp, Warning, TEXT(" ChecklistManager 동적 생성"));
	//// ChecklistManager 동적 생성
	//if (HasAuthority() && ChecklistManagerClass)
	//{
	//	FActorSpawnParameters SpawnParams;
	//	SpawnParams.bNoFail = true;
	//	SpawnParams.bDeferConstruction = false;
	//	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	//	if (ChecklistManagerClass.IsValid())
	//	{
	//		ChecklistManager = GetWorld()->SpawnActor<AChecklistManager>(
	//			ChecklistManagerClass.Get(),               // UClass*
	//			FVector::ZeroVector,
	//			FRotator::ZeroRotator,
	//			SpawnParams
	//		);
	//	}
	//	
	//	if (ChecklistManager)
	//	{
	//		ChecklistManager->SetReplicates(true); // 이걸 반드시 해야 클라에도 복제됨
	//		UE_LOG(LogTemp, Warning, TEXT("[Checklist] ChecklistManager Spawned"));
	//	}
	//}
}

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
