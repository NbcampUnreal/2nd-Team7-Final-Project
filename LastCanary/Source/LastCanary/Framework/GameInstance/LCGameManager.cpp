#include "Framework/GameInstance/LCGameManager.h"

#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Framework/GameMode/LCInGameModeBase.h"

#include "GameFramework/GameStateBase.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

void ULCGameManager::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

}

void ULCGameManager::InitGameManager()
{
	CurrentPlayerCount = 0;
	CurrentGamePlayData = FGamePlayData();
	CurrentRound = 0;
	MaxRounds = 3;
	CurrentGold = 5000; // 초기 골드 설정
	GoldHistory.Empty();
}

void ULCGameManager::StartGame()
{
	CurrentRound++;
	bIsGameStarted = true;

	InitCurrentRoundResult();
}

void ULCGameManager::InitCurrentRoundResult()
{
	CurrentGamePlayData = FGamePlayData();
	CurrentGamePlayData.Round = CurrentRound;
	CurrentGamePlayData.MapName = UGameplayStatics::GetCurrentLevelName(GetWorld(), true);
	// Player 수만큼  CurrentResult의 PlayerResource 초기화

	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GameState = World->GetGameState())
		{
			for (APlayerState* PS : GameState->PlayerArray)
			{
				const FString PlayerName = PS->GetPlayerName();

				// 빈 자원 맵으로 초기화
				CurrentGamePlayData.PlayerResult.Add(PlayerName, FResultInfo());
			}
		}
	}

	AllGamePlayData.Add(CurrentGamePlayData);
}


void ULCGameManager::EndCurrentRound()
{
	if (UWorld* World = GetWorld())
	{
		if (ALCInGameModeBase* InGameMode = Cast<ALCInGameModeBase>(World->GetAuthGameMode()))
		{

		}
	}
}

void ULCGameManager::EndGame()
{
	bIsGameStarted = false;
}

bool ULCGameManager::IsGameEnd()
{
	return CurrentRound == MaxRounds;
}

void ULCGameManager::SubmitExplorationResults(FString PlayerName, bool bIsDead, TMap<FName, int32> Results)
{
	if (CurrentGamePlayData.PlayerResult.Contains(PlayerName))
	{
		FResultInfo Result;
		Result.bIsDead = bIsDead;
		Result.PlayerResources = Results;

		CurrentGamePlayData.PlayerResult[PlayerName] = Result;
	}
}

int ULCGameManager::GetGold() const
{
	return CurrentGold;
}

void ULCGameManager::UpdateGold(FString Reason, int Amount)
{
	CurrentGold += Amount;
	GoldHistory.Add(TPair<FString, int>(Reason, Amount));

	if (CurrentGold < 0)
	{
		CurrentGold = 0;
	}

	LOG_Game(Log, TEXT("골드가 %d만큼 변경되었습니다. 현재 골드: %d"), Amount, CurrentGold);
	//PrintGoldHistory();
}

void ULCGameManager::PrintGoldHistory()
{
	LOG_Game(Log, TEXT("======= 골드 추가 및 사용 이력 =========="));
	for (auto History : GoldHistory)
	{
		LOG_Game(Log, TEXT("이유 : %s, Amount : %d"), *History.Key, History.Value);

	}
}
