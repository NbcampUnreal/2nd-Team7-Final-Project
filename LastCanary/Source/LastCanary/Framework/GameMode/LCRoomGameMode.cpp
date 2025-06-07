#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/Manager/ChecklistManager.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Kismet/GameplayStatics.h"

ALCRoomGameMode::ALCRoomGameMode()
{
	//SelectedMap = EMapType::LuinsMap;
	
	static ConstructorHelpers::FClassFinder<AChecklistManager> ChecklistBPClass(TEXT("/Game/_LastCanary/Blueprint/Framework/Manager/BP_ChecklistManager.BP_ChecklistManager"));
	if (ChecklistBPClass.Succeeded())
	{
		ChecklistManagerClass = ChecklistBPClass.Class;
	}
	
}

void ALCRoomGameMode::BeginPlay()
{
	Super::BeginPlay();
	UE_LOG(LogTemp, Warning, TEXT("ALCRoomGameMode BeginPlay"));
	// 생존자 수 초기화
	if (ALCGameState* LCGameState = GetGameState<ALCGameState>())
	{
		int32 InitialAliveCount = 0;

		for (APlayerState* PlayerState : LCGameState->PlayerArray)
		{
			if (IsValid(PlayerState) && !PlayerState->IsOnlyASpectator())
			{
				++InitialAliveCount;
			}
		}

		LCGameState->AlivePlayerCount = InitialAliveCount;
	}
}

void ALCRoomGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) 
{
	UE_LOG(LogTemp, Log, TEXT("1. InitGame 호출됨"));
	Super::InitGame(MapName, Options, ErrorMessage);
	ULCGameInstanceSubsystem* GI = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (GI)
	{
		UE_LOG(LogTemp, Log, TEXT("이전 맵에서 넘어온 플레이어 수: %d"), GI->UserNum);
		InGamePlayerNum = GI->UserNum;
	}
}

void ALCRoomGameMode::InitGameState() {
	UE_LOG(LogTemp, Log, TEXT("2. InitGameState 호출됨"));
}

void ALCRoomGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

	UE_LOG(LogTemp, Log, TEXT("4. PlayerController %s 가 심리스 트래블로 이관됨"), *C->GetName());

	ConnectedPlayers++;
	ALCGameState* LCGameState = GetGameState<ALCGameState>();
	if (!IsValid(LCGameState))
	{
		return;

	}
	if (InGamePlayerNum <= ConnectedPlayers)
	{
		UE_LOG(LogTemp, Log, TEXT("모든 플레이어 연결 완료!"));
		StartGame();
	}
}

void ALCRoomGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	UE_LOG(LogTemp, Log, TEXT("5. PostSeamlessTravel 호출됨"));
}




void ALCRoomGameMode::StartGame()
{	
	UE_LOG(LogTemp, Warning, TEXT(" ChecklistManager 동적 생성"));
	// ChecklistManager 동적 생성
	if (HasAuthority() && ChecklistManagerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.bNoFail = true;
		SpawnParams.bDeferConstruction = false;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		ChecklistManager = GetWorld()->SpawnActor<AChecklistManager>(ChecklistManagerClass, FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
		if (ChecklistManager)
		{
			ChecklistManager->SetReplicates(true); // 이걸 반드시 해야 클라에도 복제됨
			UE_LOG(LogTemp, Warning, TEXT("[Checklist] ChecklistManager Spawned"));
		}
	}
}

void ALCRoomGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	UE_LOG(LogTemp, Warning, TEXT("PostLogin 시작"));
}

APlayerController* ALCRoomGameMode::Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	return Super::Login(NewPlayer, InRemoteRole, Portal, Options, UniqueId, ErrorMessage);

}

void ALCRoomGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

}
