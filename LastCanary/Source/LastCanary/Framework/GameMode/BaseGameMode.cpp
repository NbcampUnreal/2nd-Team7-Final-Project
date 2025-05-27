// Fill out your copyright notice in the Description page of Project Settings.


#include "Framework/GameMode/BaseGameMode.h"
#include "Framework/GameInstance/LCGameInstance.h"
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
				//PlayerController->Client_UpdatePlayers();
			}
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("ABaseGameMode::PostLogin : GameInstance Cast Failed."))
	}
}

bool ABaseGameMode::HasCharacterSpawner() const
{
	//for (TActorIterator<ACharacterSpawner> It(GetWorld()); It; ++It)
	//{
	//	return true;
	//}
	return false;
}

void ABaseGameMode::SpawnPlayerCharacter(APlayerController* Controller)
{
	// 하위 게임모드에서 구현
}

void ABaseGameMode::HandlePlayerDeath(APlayerController* Controller, const FString& AttackerName)
{
	// 게임모드에 특정 함수 호출
	// 타이머 + 흑백UI
	SpawnPlayerCharacter(Controller);

	// Kill Log
	for (FConstPlayerControllerIterator Iterator = GetWorld()->GetPlayerControllerIterator(); Iterator; ++Iterator)
	{
		if (ALCPlayerController* PlayerController = Cast<ALCPlayerController>(Iterator->Get()))
		{
			//UE_LOG(LogTemp, Warning, TEXT("ABaseGameMode::HandlePlayerDeath"))
			//	PlayerController->Client_PlayerDead(GetPlayerNameFromGameInstance(Controller), AttackerName);
		}
	}
}

// Helper function to get player name from GameInstance
FString ABaseGameMode::GetPlayerNameFromGameInstance(APlayerController* Controller)
{
	if (Controller == nullptr)
	{
		return TEXT("Unknown");
	}

	if (const ULCGameInstance* GameInstance = Cast<ULCGameInstance>(GetGameInstance()))
	{
		FString PlayerName = GameInstance->GetPlayerName();
		if (!PlayerName.IsEmpty())
		{
			return PlayerName;
		}
	}

	// Fallback to PlayerState name if available
	if (Controller->PlayerState)
	{
		return Controller->PlayerState->GetPlayerName();
	}

	return FString::Printf(TEXT("Player_%d"), Controller->GetUniqueID());
}