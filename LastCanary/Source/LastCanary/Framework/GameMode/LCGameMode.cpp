#include "Framework/GameMode/LCGameMode.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "LastCanary.h"
#include "GameFramework/GameSession.h"

void ALCGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void ALCGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (CurrentPlayerNum >= MaxPlayerNum)
	{
		GameSession->KickPlayer(NewPlayer, FText::FromString("Room Is Full !!"));
		return;
	}

	CachingNewPlayer(NewPlayer);
}

void ALCGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		RemoveCachedPlayer(PC);
	}
}

void ALCGameMode::KickPlayer(const FSessionPlayerInfo& SessionInfo, const FText& KickReason)
{
	APlayerController* TargetPC = nullptr;
	for (APlayerController* PC : AllPlayerControllers)
	{
		if (!PC || !PC->PlayerState)
		{
			continue;
		}

		if (PC->PlayerState->GetPlayerName() == SessionInfo.PlayerName)
		{
			TargetPC = PC;
			break;
		}
	}

	if (!TargetPC)
	{
		LOG_Server_ERROR(TEXT("KickPlayer: 이름이 %s 인 플레이어를 찾을 수 없습니다."), *SessionInfo.PlayerName);
		return;
	}

	GameSession->KickPlayer(TargetPC, KickReason);

	//RemoveCachedPlayer(TargetPC);
}

void ALCGameMode::CachingNewPlayer(APlayerController* NewPlayer)
{
	CurrentPlayerNum++;

	AllPlayerControllers.Add(NewPlayer);

	if (APlayerState* PS = NewPlayer->PlayerState)
	{
		FSessionPlayerInfo SessionInfo;
		SessionInfo.PlayerName = PS->GetPlayerName();
		SessionInfo.bIsPlayerReady = false;
		SessionPlayerInfos.Add(SessionInfo);
	}

	UpdatePlayers();
}

void ALCGameMode::RemoveCachedPlayer(APlayerController* PC)
{
	CurrentPlayerNum--;

	AllPlayerControllers.Remove(PC);

	SessionPlayerInfos.RemoveAll
	(
		[&](const FSessionPlayerInfo& Info)
		{
			return Info.PlayerName == PC->PlayerState->GetPlayerName();
		}
	);

	UpdatePlayers();
}

void ALCGameMode::UpdatePlayers()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
		{
			PlayerController->Client_UpdatePlayerList(SessionPlayerInfos);
		}
	}
}

void ALCGameMode::SetPlayerInfo(const FSessionPlayerInfo& RequestInfo)
{
	for (FSessionPlayerInfo& Info : SessionPlayerInfos)
	{
		if (RequestInfo.PlayerName == Info.PlayerName)
		{
			Info = RequestInfo;

			LOG_Server_ERROR(
				TEXT("PlayerInfo Changed Name : %s, Ready : %s"),
				*RequestInfo.PlayerName,
				RequestInfo.bIsPlayerReady ? TEXT("True") : TEXT("False")
			);
		}
	}

	UpdatePlayers();
}

void ALCGameMode::ShowLoading()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
		{
			PlayerController->Client_ShowLoading();
		}
	}
}


void ALCGameMode::SpawnPlayerCharacter(APlayerController* Controller)
{
	// 하위 게임모드에서 구현
}

void ALCGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (CurrentPlayerNum >= MaxPlayerNum)
	{
		ErrorMessage = TEXT("Room Is Full !!");
	}
}


void ALCGameMode::TravelMapBySoftPath(FString SoftPath)
{
	// 1) SoftObjectPath: "/Game/.../MainLevel.MainLevel"

	// 2) 패키지 경로만 분리: "/Game/.../MainLevel"
	const FString PackageName = FPackageName::ObjectPathToPackageName(SoftPath);

	// 3) ?listen 붙여서 최종 ServerTravel URL생성
	const FString TravelURL = PackageName + TEXT("?listen");
	UE_LOG(LogTemp, Log, TEXT("Try Server Travel By SoftPath. Traveling to: %s"), *TravelURL);

	GetWorld()->ServerTravel(TravelURL, true);
}

void ALCGameMode::TravelMapByPath(FString Path)
{
	const FString TravelURL = Path + TEXT("?listen");
	UE_LOG(LogTemp, Log, TEXT("Try Server Travel By Path. Traveling to: %s"), *TravelURL);

	GetWorld()->ServerTravel(TravelURL, true);
}

bool ALCGameMode::IsAllPlayersReady() const
{
	if (SessionPlayerInfos.Num() == 0)
		return false;

	for (const FSessionPlayerInfo& Info : SessionPlayerInfos)
	{
		if (!Info.bIsPlayerReady)
		{
			return false;
		}
	}
	return true;
}
