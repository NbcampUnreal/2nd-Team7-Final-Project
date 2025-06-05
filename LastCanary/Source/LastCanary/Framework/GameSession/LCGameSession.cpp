#include "Framework/GameSession/LCGameSession.h"
#include "Framework/GameMode/LCGameMode.h"
#include "Engine/World.h"
#include "LastCanary.h"

FString ALCGameSession::ApproveLogin(const FString& Options)
{
	return Super::ApproveLogin(Options);

}

void ALCGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
    Super::RegisterPlayer(NewPlayer, UniqueId, bWasFromInvite);

}

void ALCGameSession::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	LOG_Server(Log, TEXT("Player LogIned At Session !!"));

}

void ALCGameSession::BeginPlay()
{
    Super::BeginPlay();

    LOG_Server(Log, TEXT("Starting LC Game Session"));
}



void ALCGameSession::NotifyLogout(const APlayerController* ExitingPlayer)
{
    Super::NotifyLogout(ExitingPlayer);

}

void ALCGameSession::UnregisterPlayer(const APlayerController* ExitingPlayer)
{
    Super::UnregisterPlayer(ExitingPlayer);

}

void ALCGameSession::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

}

bool ALCGameSession::KickPlayer(APlayerController* KickedPlayer, const FText& KickReason)
{
    return Super::KickPlayer(KickedPlayer, KickReason);
}
