#include "Framework/GameMode/LCGameMode.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "Framework/GameInstance/LCGameManager.h"
#include "Framework/PlayerController/LCPlayerController.h"

#include "GameFramework/GameSession.h"
#include "Kismet/GameplayStatics.h"

void ALCGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (CurrentPlayerNum >= MaxPlayerNum)
	{
		ErrorMessage = TEXT("Room Is Full !!");
		return;
	}

	if (!CanJoinSessionMap())
	{
		ErrorMessage = TEXT("게임이 이미 시작되어 참여할 수 없습니다.");
	}
}

void ALCGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>())
	{
		LCGM->CurrentPlayerCount++;
	}

}

void ALCGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	UpdateSessionState();
}

void ALCGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

}

void ALCGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	CachingNewPlayer(NewPlayer);

}

void ALCGameMode::StartPlay()
{
	Super::StartPlay();

}

void ALCGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void ALCGameMode::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	if (APlayerController* PC = Cast<APlayerController>(Exiting))
	{
		RemoveCachedPlayer(PC);
	}

	if (ULCGameManager* LCGM = GetGameInstance()->GetSubsystem<ULCGameManager>())
	{
		LCGM->CurrentPlayerCount--;
	}
}

bool ALCGameMode::CanJoinSessionMap()
{
	// 1) 현재 월드의 레벨 이름을 가져옴
	FString MapName = UGameplayStatics::GetCurrentLevelName(this, true);

	// 2) 허용된 레벨 이름 확인
	static const TArray<FString> AllowedJoinMaps =
	{
		TEXT("BaseCamp")
	};

	return AllowedJoinMaps.Contains(MapName);
}

void ALCGameMode::UpdateSessionState()
{
	if (ULCGameInstance* LCGI = Cast<ULCGameInstance>(GetGameInstance()))
	{
		FString MapName = UGameplayStatics::GetCurrentLevelName(this, /*bRemovePIEPrefix=*/ true);

		static const TArray<FString> AllowedJoinMaps = {
			TEXT("BaseCamp")
		};

		if (AllowedJoinMaps.Contains(MapName))
		{
			// BaseCamp 로 돌아왔으니 세션 검색/조인 허용 재활성화
			LCGI->UpdateSession(true, true, MaxPlayerNum);
		}
		else
		{
			// InGameLevel 에서는 세션 숨기기 & 조인 금지
			LCGI->UpdateSession(false, false, 0);
		}
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
}

void ALCGameMode::CachingNewPlayer(APlayerController* NewPlayer)
{
	//bool isInSessionPlayerInfos = false;
	if (AllPlayerControllers.Contains(NewPlayer))
	{
		return;
	}

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
	for (APlayerController* PC : AllPlayerControllers)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(PC))
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

void ALCGameMode::HideLoading()
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
		{
			PlayerController->Client_ShowLoading();
		}
	}
}

void ALCGameMode::SendMessageToAllPC(const FString& Message)
{
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
		{
			PlayerController->Client_ReceiveMessageFromGM(Message);
		}
	}
}

void ALCGameMode::TravelMapBySoftPath(FString SoftPath)
{
	// 1) SoftObjectPath: "/Game/.../MainLevel.MainLevel"

	// 2) 패키지 경로만 분리: "/Game/.../MainLevel"
	const FString PackageName = FPackageName::ObjectPathToPackageName(SoftPath);

	// 3) ?listen 붙여서 최종 ServerTravel URL생성
	const FString TravelURL = PackageName + TEXT("?listen");
	LOG_Server_WARNING(TEXT("Try Server Travel By SoftPath. Traveling to: %s"), *TravelURL);

	GetWorld()->ServerTravel(TravelURL, true);
}

void ALCGameMode::TravelMapByPath(FString Path)
{
	const FString TravelURL = Path + TEXT("?listen");
	LOG_Server_WARNING(TEXT("Try Server Travel By Path. Traveling to: %s"), *TravelURL);

	GetWorld()->ServerTravel(TravelURL, true);
}

bool ALCGameMode::IsAllPlayersReady() const
{
	if (SessionPlayerInfos.Num() == 0)
	{
		return false;
	}

	for (const FSessionPlayerInfo& Info : SessionPlayerInfos)
	{
		if (!Info.bIsPlayerReady)
		{
			return false;
		}
	}
	return true;
}
