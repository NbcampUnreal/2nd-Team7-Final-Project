#include "Framework/GameMode/BaseGameMode.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "LastCanary.h"
#include "GameFramework/GameSession.h"

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (const auto CastedGameInstance = Cast<ULCGameInstance>(GetGameInstance()))
	{
		//SpawnPlayerCharacter(NewPlayer);

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
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ABaseGameMode::PostLogin : GameInstance Cast Failed."))
	}
}

void ABaseGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		AllPlayerControllers.Remove(PC);

		FString LeavingPlayerName = PC->PlayerState->GetPlayerName();
		SessionPlayerInfos.RemoveAll(
			[&](const FSessionPlayerInfo& Info)
			{
				return Info.PlayerName == LeavingPlayerName;
			}
		);
	}

	UpdatePlayers();
}

void ABaseGameMode::KickPlayer(const FSessionPlayerInfo& SessionInfo)
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

	const FText KickReason = FText::FromString(TEXT("호스트에 의해 강퇴되었습니다."));
	GameSession->KickPlayer(TargetPC, KickReason);

	AllPlayerControllers.Remove(TargetPC);

	SessionPlayerInfos.RemoveAll(
		[&](const FSessionPlayerInfo& Info)
		{
			return Info.PlayerName == SessionInfo.PlayerName;
		}
	);

	UpdatePlayers();
}

void ABaseGameMode::UpdatePlayers()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
		{
			PlayerController->Client_UpdatePlayerList(SessionPlayerInfos);
		}
	}
}

void ABaseGameMode::SetPlayerInfo(const FSessionPlayerInfo& RequestInfo)
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


void ABaseGameMode::SpawnPlayerCharacter(APlayerController* Controller)
{
	// 하위 게임모드에서 구현
}


void ABaseGameMode::TravelMapBySoftPath(FString SoftPath)
{
	// 1) SoftObjectPath: "/Game/.../MainLevel.MainLevel"

	// 2) 패키지 경로만 분리: "/Game/.../MainLevel"
	const FString PackageName = FPackageName::ObjectPathToPackageName(SoftPath);

	// 3) ?listen 붙여서 최종 ServerTravel URL생성
	const FString TravelURL = PackageName + TEXT("?listen");
	UE_LOG(LogTemp, Log, TEXT("Try Server Travel By SoftPath. Traveling to: %s"), *TravelURL);

	GetWorld()->ServerTravel(TravelURL, true);
}

void ABaseGameMode::TravelMapByPath(FString Path)
{
	const FString TravelURL = Path + TEXT("?listen");
	UE_LOG(LogTemp, Log, TEXT("Try Server Travel By Path. Traveling to: %s"), *TravelURL);

	GetWorld()->ServerTravel(TravelURL, true);
}
