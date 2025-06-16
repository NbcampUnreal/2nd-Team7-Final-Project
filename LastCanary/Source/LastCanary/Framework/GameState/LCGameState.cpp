#include "Framework/GameState/LCGameState.h"
#include "Net/UnrealNetwork.h"
#include "Framework/GameInstance/LCGameInstanceSubsystem.h"
#include "UI/UIElement/ChecklistWidget.h"
#include "UI/Manager/LCUIManager.h"
#include "Character/BasePlayerState.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Framework/PlayerController/LCRoomPlayerController.h"
#include "EngineUtils.h"
#include "Framework/GameMode/LCRoomGameMode.h"
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
		AChecklistManager* ChecklistManager = MyGameMode->ChecklistManager;
		if (!ChecklistManager)
		{
			LOG_Frame_WARNING(TEXT("체크리스트가 없음"));
			return;
		}

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (ALCRoomPlayerController* PC = Cast<ALCRoomPlayerController>(*It))
			{
				UE_LOG(LogTemp, Log, TEXT("검사 중인 컨트롤러 이름: %s"), *PC->GetName());
				PC->Client_StartChecklist(ChecklistManager);
			}
		}
	}
	else
	{
		LOG_Frame_WARNING(TEXT("Not all players have escaped yet."));
	}
}
