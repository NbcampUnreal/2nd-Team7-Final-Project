#include "Framework/GameMode/LCInGameModeBase.h"

#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerState/LCPlayerState.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"

#include "Framework/Manager/ChecklistManager.h"

#include "DataTable/MapDataRow.h"

#include "Kismet/GameplayStatics.h"


ALCInGameModeBase::ALCInGameModeBase()
{

}

void ALCInGameModeBase::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	LOG_Server(Log, TEXT("1. InitGame 호출됨"));

}

void ALCInGameModeBase::InitGameState()
{
	Super::InitGameState();
	LOG_Server(Log, TEXT("2. InitGameState 호출됨"));

}

void ALCInGameModeBase::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();
	LOG_Server(Log, TEXT("3. Post Seamless Travel 호출됨"));

}

void ALCInGameModeBase::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

}

void ALCInGameModeBase::StartPlay()
{
	Super::StartPlay();
	LOG_Server(Log, TEXT("[ALCInGameModeBase] Host 플레이어가 HandleSeamlessTravelPlayer 이후 Start Play 호출!!"));
}

void ALCInGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	LOG_Server(Log, TEXT("[ALCInGameModeBase] BeginPlay"));

	CreateCheckListManager();

}

void ALCInGameModeBase::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	LOG_Server_WARNING(TEXT("Player : %s 시작함!!"), *NewPlayer->PlayerState->GetPlayerName());

	// 연결된 클라이언트에게 ChecklistManager의 Owner 지정
	if (ChecklistManager && NewPlayer)
	{
		ChecklistManager->SetOwner(NewPlayer);
	}

	if (ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>())
	{
		if (LCGM->CurrentPlayerCount == CurrentPlayerNum)
		{
			OnAllPlayersJoined();
		}
	}
}

void ALCInGameModeBase::OnAllPlayersJoined()
{	
	LOG_Server(Log, TEXT("모든 플레이어가 연결되었습니다. 게임을 시작합니다."));

	StartGame();
}

void ALCInGameModeBase::StartGame()
{
	ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>();
	if (!LCGM)
	{
		return;
	}

	LCGM->StartGame();

	InitGameState(LCGM->GetPlayerCount());

	ShowGameLevelInfo();

}

void ALCInGameModeBase::GameEnd()
{


}

void ALCInGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (ALCGameState* LCGameState = GetGameState<ALCGameState>())
	{
		LCGameState->RemoveAlivePlayer();
	}
}

void ALCInGameModeBase::InitGameState(int PlayerCount)
{
	// 생존자 수 초기화
	if (ALCGameState* LCGameState = GetGameState<ALCGameState>())
	{
		LCGameState->AlivePlayerCount = PlayerCount;
	}
}

void ALCInGameModeBase::CreateCheckListManager()
{
	// ChecklistManager 동적 생성
	if (ChecklistManagerClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		if (IsValid(ChecklistManagerClass))
		{
			ChecklistManager = GetWorld()->SpawnActor<AChecklistManager>(
				ChecklistManagerClass.Get(),               // UClass*
				FVector::ZeroVector,
				FRotator::ZeroRotator,
				SpawnParams
			);
		}

		if (ChecklistManager)
		{
			UE_LOG(LogTemp, Warning, TEXT("[Checklist] ChecklistManager Spawned"));
		}
	}
}

void ALCInGameModeBase::ShowGameLevelInfo()
{
	// Seamless Travel 이후 현재 도착한 Level에 대한 정보 및 목표 표시
	ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>();
	if (!LCGM)
	{
		LOG_Game_WARNING(TEXT("Fail Get LCGameManager!"));
		return;
	}
	ULCGameInstanceSubsystem* Subsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>();
	if (!IsValid(Subsystem))
	{
		LOG_Game_WARNING(TEXT("Fail Get GameInstance Subsystem!"));
		return;
	}

	if (LCGM->CurrentPlayerCount == CurrentPlayerNum)
	{
		LOG_Game(Log, TEXT("모든 플레이어가 연결되었습니다. 게임을 시작합니다."));

		FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);
		const FMapDataRow* MapData = Subsystem->GetMapDataByRowName(FName(MapName));
		if (!MapData)
		{
			return;
		}

		// 0.3초 뒤에 SendMessageToAllPC 호출
		FTimerHandle TimerHandle;
		FTimerDelegate TimerDel;
		TimerDel.BindLambda([this, MapData]()
			{
				for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
				{
					if (ALCInGamePlayerController* InGamePC = Cast<ALCInGamePlayerController>(Iterator->Get()))
					{
						InGamePC->Client_ShowLevelInfo(MapData->MapID);
					}
				}
			});

		GetWorldTimerManager().SetTimer(
			TimerHandle,
			TimerDel,
			0.3f,   // Delay
			false   // Loop? false = 한 번만 실행
		);

	}
}
