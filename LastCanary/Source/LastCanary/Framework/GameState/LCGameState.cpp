#include "Framework/GameState/LCGameState.h"
#include "Character/BasePlayerState.h"
#include "Framework/PlayerController/LCInGamePlayerController.h"
#include "Framework/GameMode/LCInGameModeBase.h"

#include "LastCanary.h"

ALCGameState::ALCGameState()
{
	AlivePlayerCount = 0;
}

void ALCGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void ALCGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();

}

void ALCGameState::BeginPlay()
{
	Super::BeginPlay();

}

void ALCGameState::InitMyGameState(int PlayerCount)
{
	TotalPlayerCount = PlayerCount;
	AlivePlayerCount = PlayerCount;
	DeathPlayerCount = 0;
	EscapedPlayerCount = 0;
}

void ALCGameState::OnGameStart()
{

}

void ALCGameState::PostInitializeComponents()
{
	Super::PostInitializeComponents();

}

void ALCGameState::AddAlivePlayer()
{
	if (HasAuthority())
	{
		++AlivePlayerCount;
		// 필요 시 Broadcast
	}
}

void ALCGameState::RemoveAlivePlayer()
{
	if (HasAuthority() && AlivePlayerCount > 0)
	{
		--AlivePlayerCount;
		// 필요 시 Broadcast
	}
}

void ALCGameState::OnRep_AlivePlayerCount()
{
	// 클라이언트 HUD 업데이트 등
	UE_LOG(LogTemp, Log, TEXT("AlivePlayerCount updated: %d"), AlivePlayerCount);
}

void ALCGameState::OnPlayerDeath(APlayerState* DeadPlayer)
{
	DeathPlayerCount++;
}

void ALCGameState::OnPlayerEscapedGate(APlayerState* EscapedPlayer)
{
	EscapedPlayerCount++;
}

void ALCGameState::MarkPlayerAsEscaped(APlayerState* EscapedPlayer)
{
	if (!PlayerArray.Contains(EscapedPlayer))
	{
		LOG_Frame_WARNING(TEXT("PlayerArray에 포함되지 않은 PlayerState입니다: %s"), *GetNameSafe(EscapedPlayer));
	}
	else
	{
		LOG_Frame_WARNING(TEXT("포함된 PlayerState입니다: %s"), *GetNameSafe(EscapedPlayer));
	}

	if (ABasePlayerState* BasePlayerState = Cast<ABasePlayerState>(EscapedPlayer))
	{
		BasePlayerState->bHasEscaped = true;

		if (BasePlayerState->CurrentState == EPlayerState::Dead)
		{
			OnPlayerDeath(EscapedPlayer);
		}
		else
		{
			OnPlayerEscapedGate(EscapedPlayer);
		}

		LOG_Frame_WARNING(TEXT("→ MarkAsEscaped 완료: %s | bHasEscaped = %s"),
			*BasePlayerState->GetPlayerName(), BasePlayerState->bHasEscaped ? TEXT("TRUE") : TEXT("FALSE"));
	}

	RemoveAlivePlayer(); // AliveCount--

	CheckGameEndCondition();
}

void ALCGameState::CheckGameEndCondition()
{
	if (!HasAuthority())
	{
		return;
	}

	LOG_Frame_WARNING(TEXT("=== Escape Status Check ==="));
	for (APlayerState* PS : PlayerArray)
	{
		if (const ABasePlayerState* BasePS = Cast<ABasePlayerState>(PS))
		{
			LOG_Frame_WARNING(TEXT("[Player] %s - Escaped: %s"), *BasePS->GetPlayerName(), BasePS->bHasEscaped ? TEXT("TRUE") : TEXT("FALSE"));
		}
		else
		{
			LOG_Frame_WARNING(TEXT("[Player] Unknown PlayerState: %s"), *PS->GetName());
		}
	}

	// 모든 PlayerState를 확인하여 전원 탈출했는지 확인
	bool bAllEscaped = true;

	for (APlayerState* PS : PlayerArray)
	{
		if (const ABasePlayerState* BasePS = Cast<ABasePlayerState>(PS))
		{
			if (BasePS->bHasEscaped == false)
			{
				bAllEscaped = false;
				break;
			}
		}
	}

	if (bAllEscaped)
	{
		if (EscapedPlayerCount == 0)
		{
			OnLoseGame();
		}
		else
		{
			OnEscapedGame();
		}
	}
	else
	{
		LOG_Frame_WARNING(TEXT("Not all players have escaped yet."));
	}
}

void ALCGameState::OnLoseGame()
{
	LOG_Frame_WARNING(TEXT("All players have escaped! Showing checklist now."));
	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (!GameModeBase)
	{
		LOG_Frame_WARNING(TEXT("게임모드 가져오기 오류"));
		return;
	}
	ALCInGameModeBase* MyGameMode = Cast<ALCInGameModeBase>(GameModeBase);
	if (!MyGameMode)
	{
		LOG_Frame_WARNING(TEXT("게임모드 캐스팅 오류"));
		return;
	}

	MyGameMode->LoseGame();
}

void ALCGameState::OnEscapedGame()
{
	LOG_Frame_WARNING(TEXT("All players have escaped! Showing checklist now."));
	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (!GameModeBase)
	{
		LOG_Frame_WARNING(TEXT("게임모드 가져오기 오류"));
		return;
	}
	ALCInGameModeBase* MyGameMode = Cast<ALCInGameModeBase>(GameModeBase);
	if (!MyGameMode)
	{
		LOG_Frame_WARNING(TEXT("게임모드 캐스팅 오류"));
		return;
	}

	MyGameMode->ClearGame();
}