#include "Framework/GameInstance/LCGameManager.h"

#include "UI/Manager/LCUIManager.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameMode/LCRoomGameMode.h"
#include "Kismet/GameplayStatics.h"

#include "LastCanary.h"

void ULCGameManager::Initialize(FSubsystemCollectionBase& Collection)
{
}

void ULCGameManager::InitGameManager()
{
	CurrentPlayerCount = 0;
	CurrentRound = 0;
	MaxRounds = 3;
	CurrentGold = 5000; // 초기 골드 설정
}

void ULCGameManager::StartGame()
{

}

void ULCGameManager::EndCurrentRound()
{

}

int ULCGameManager::GetGold() const
{
	return CurrentGold;
}

void ULCGameManager::AddGold(int Amount)
{
	CurrentGold += Amount;
	if (CurrentGold < 0)
	{
		CurrentGold = 0; // 골드가 음수가 되지 않도록 보장
	}
	LOG_Server(Log, TEXT("골드가 %d만큼 추가되었습니다. 현재 골드: %d"), Amount, CurrentGold);
}
