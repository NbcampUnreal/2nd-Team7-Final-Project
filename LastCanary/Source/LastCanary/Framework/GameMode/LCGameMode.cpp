#include "Framework/GameMode/LCGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/PlayerController/LCPlayerController.h"
#include "GameFramework/GameSession.h"
#include "EngineUtils.h"
#include "Actor/PlayerChecker.h"

#include "Framework/GameInstance/LCGameInstanceSubsystem.h"

#include "LastCanary.h"

void ALCGameMode::PreLogin(const FString& Options, const FString& Address, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	if (CurrentPlayerNum >= MaxPlayerNum)
	{
		ErrorMessage = TEXT("Room Is Full !!");
	}
}

void ALCGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

}

void ALCGameMode::PostSeamlessTravel()
{
	Super::PostSeamlessTravel();

	// 레벨에 존재하는 PlayerChecker 재초기화
	for (TActorIterator<APlayerChecker> It(GetWorld()); It; ++It)
	{
		It->InitializeChecker(); 
		It->Server_OpenDoors();
	}
}

void ALCGameMode::HandleSeamlessTravelPlayer(AController*& C)
{
	Super::HandleSeamlessTravelPlayer(C);

}

void ALCGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	CachingNewPlayer(NewPlayer);

	for (TActorIterator<APlayerChecker> It(GetWorld()); It; ++It)
	{
		It->Server_OpenDoors(); // 강제로 다시 열어줌 S
	}
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
	// SeamlessTravel시에 아직 이동하지 않은 PlayerController가 잡혀서 World에 3개의 Controller가 있는 경우가 있어서
	// 캐싱한 PlayerController 에게 만 RPC 전송
	for (APlayerController* PC : AllPlayerControllers)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(PC))
		{
			PlayerController->Client_UpdatePlayerList(SessionPlayerInfos);
		}
	}

	if (ULCGameInstanceSubsystem* GISubsystem = GetGameInstance()->GetSubsystem<ULCGameInstanceSubsystem>())
	{
		GISubsystem->SetUserNum(SessionPlayerInfos.Num());
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

	// 준비 상태 확인 및 문 열고 닫기 처리
	int32 Connected = 0;
	int32 Ready = 0;

	for (const FSessionPlayerInfo& Info : SessionPlayerInfos)
	{
		if (!Info.PlayerName.IsEmpty())
		{
			Connected++;
			if (Info.bIsPlayerReady)
			{
				Ready++;
			}
		}
	}

	for (TActorIterator<APlayerChecker> It(GetWorld()); It; ++It)
	{
		if (Connected > 0 && Connected == Ready)
		{
			It->Server_CloseDoors();
		}
		else
		{
			It->Server_OpenDoors();
		}
	}
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


void ALCGameMode::SpawnPlayerCharacter(APlayerController* Controller)
{
	// 하위 게임모드에서 구현
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
