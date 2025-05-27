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

		for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
		{
			if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
			{
				PlayerController->Client_UpdatePlayers();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ABaseGameMode::PostLogin : GameInstance Cast Failed."))
	}
}

void ABaseGameMode::SpawnPlayerCharacter(APlayerController* Controller)
{
	// 하위 게임모드에서 구현
}
