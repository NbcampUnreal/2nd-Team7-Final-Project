#include "Framework/GameMode/LCInGameModeBase.h"

#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameState/LCGameState.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"
#include "Framework/Manager/ChecklistManager.h"

#include "Character/BasePlayerState.h"

#include "DataTable/BossMonsterRow.h"
#include "DataTable/MapDataRow.h"
#include "Actor/LCBossSpawner.h"

#include "EngineUtils.h"
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

	InitLCGameMode();
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

void ALCInGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (ALCGameState* LCGameState = GetGameState<ALCGameState>())
	{
		LCGameState->RemoveAlivePlayer();
	}
}

void ALCInGameModeBase::OnAllPlayersJoined()
{	
	LOG_Server(Log, TEXT("모든 플레이어가 연결되었습니다. 게임을 시작합니다."));

	FTimerHandle StartTimerHandle;
	FTimerDelegate TimerDel;

	TimerDel.BindLambda([this]()
		{
			HideLoading();
			StartGame();
		});

	GetWorldTimerManager().SetTimer(StartTimerHandle, TimerDel, ShowLoadingDelay, false);
}

void ALCInGameModeBase::StartGame()
{
	LOG_Server(Log, TEXT("InGame Mode Start Game!!"));

	ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>();
	if (!IsValid(LCGM))
	{
		LOG_Server_ERROR(TEXT("LC GameManager Casting Fail!!"));
		return;
	}

	LCGM->StartGame();

	ALCGameState* LCGS = GetGameState<ALCGameState>();
	if (!IsValid(LCGS))
	{
		LOG_Server_ERROR(TEXT("LC GameManager Casting Fail!!"));
		return;
	}

	LCGS->OnGameStart();
	LCGS->InitMyGameState(LCGM->GetPlayerCount());

	CreateBossMonster();

	CreateCheckListManager();

	ShowGameLevelInfo();

	PlayerLifeTimerStart();
}

void ALCInGameModeBase::ClearGame()
{
	LOG_Server(Log, TEXT("InGame Mode Clear Game!!"));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALCInGamePlayerController* PC = Cast<ALCInGamePlayerController>(*It))
		{
			APawn* PlayerPawn = PC->GetPawn();
			if (IsValid(PlayerPawn))
			{
				PlayerPawn->SetActorLocation(SafeLocation); // 안전 지점 이동
				PlayerPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f)); // 원하는 방향으로 회전
			}

			if (CurrentBossMonsterData)
			{
				if (CurrentBossMonsterData->CheckListTable)
				{
					PC->Client_OnEscapeGate(CurrentBossMonsterData->CheckListTable);
				}
			}
		}
	}

}

void ALCInGameModeBase::LoseGame()
{
	LOG_Server(Log, TEXT("InGame Mode Lose Game!!"));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALCInGamePlayerController* PC = Cast<ALCInGamePlayerController>(*It))
		{
			APawn* PlayerPawn = PC->GetPawn();
			if (IsValid(PlayerPawn))
			{
				PlayerPawn->SetActorLocation(SafeLocation); // 안전 지점 이동
				PlayerPawn->SetActorRotation(FRotator(0.f, 0.f, 0.f)); // 원하는 방향으로 회전
			}

			PC->Client_OnGameLose();
		}
	}
}

void ALCInGameModeBase::EndGame()
{
	LOG_Game(Log, TEXT("GameEnd"));

	for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
	{
		if (ALCInGamePlayerController* PC = Cast<ALCInGamePlayerController>(*It))
		{
			PC->Client_OnGameEnd();
		}
	}
}


void ALCInGameModeBase::InitLCGameMode()
{
	InitBossSpawner();
	CreateBossMonster();
}

void ALCInGameModeBase::InitBossSpawner()
{
	for (TActorIterator<ALCBossSpawner> It(GetWorld()); It; ++It)
	{
		BossSpawner = *It;
		LOG_Game(Log, TEXT("Boss Spawner 찾음 !! 초기화 작업 완료!"));
	}
}

void ALCInGameModeBase::CreateBossMonster()
{
	if (!IsValid(BossDataTable))
	{
		LOG_Game_ERROR(TEXT("Not Found BossDataTable"));
		return;
	}

	TArray<FBossMonsterRow*> AllBosses;
	static const FString Ctx = TEXT("InGameMode-SelectRandomBoss");
	BossDataTable->GetAllRows(Ctx, AllBosses);
	if (AllBosses.Num() > 0)
	{
		int32 RandomIdx = FMath::RandRange(0, AllBosses.Num() - 1);
		CurrentBossMonsterData = AllBosses[RandomIdx];
		LOG_Game_WARNING(TEXT("InGameMode Selected Boss : %s"), *CurrentBossMonsterData->BossName.ToString());
	}

	if (IsValid(BossSpawner))
	{
		BossSpawner->SpawnBoss(CurrentBossMonsterData->BossClass);
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

		ChecklistManager->InitCheckListManager(CurrentBossMonsterData->CheckListTable);

		if (ChecklistManager)
		{
			LOG_Game(Log, TEXT("[Checklist] ChecklistManager Spawned"));
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

void ALCInGameModeBase::PlayerLifeTimerStart()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (APlayerController* PC = Cast<APlayerController>(Iterator->Get()))
		{
			if (ABasePlayerState* BasePS = Cast<ABasePlayerState>(PC->PlayerState))
			{
				BasePS->StartSurviveTimer();
			}
		}
	}
}
