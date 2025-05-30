#include "Framework/GameMode/BaseGameMode.h"

#include "Framework/GameInstance/LCGameInstance.h"
#include "GameFramework/PlayerStart.h"
#include "Framework/PlayerController/LCPlayerController.h"

ABaseGameMode::ABaseGameMode()
{
}

void ABaseGameMode::BeginPlay()
{
	Super::BeginPlay();

}

void ABaseGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (const auto CastedGameInstance = Cast<ULCGameInstance>(GetGameInstance()))
	{
		SpawnPlayerCharacter(NewPlayer);

		AllPlayerControllers.Add(NewPlayer);

		FSessionPlayerInfo SessionInfo;
		if (APlayerState* PS = NewPlayer->PlayerState)
		{
			SessionInfo.PlayerName = PS->GetPlayerName();
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
		FString PlayerName = PC->PlayerState->GetPlayerName();

		SessionPlayerInfos.RemoveAll(
			[&](const FSessionPlayerInfo& Info)
			{
				return Info.PlayerName == PlayerName;
			}
		);
	}

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
