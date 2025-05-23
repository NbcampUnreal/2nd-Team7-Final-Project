#include "Framework/GameSession/LCGameSession.h"

#include "OnlineSessionSettings.h"
#include "OnlineSubsystem.h"
#include "OnlineSubsystemUtils.h"
#include "Interfaces/OnlineSessionInterface.h"

ALCGameSession::ALCGameSession()
{
}

void ALCGameSession::BeginPlay()
{
	Super::BeginPlay();

	UE_LOG(LogTemp, Display, TEXT("Starting LC Game Session"));
}

void ALCGameSession::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	//DestroySession();
}

bool ALCGameSession::ProcessAutoLogin()
{
	return true;
}

FString ALCGameSession::ApproveLogin(const FString& Options)
{
	return FString();
}

void ALCGameSession::PostLogin(APlayerController* NewPlayer)
{
}

void ALCGameSession::NotifyLogout(const APlayerController* ExitingPlayer)
{
}

void ALCGameSession::RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite)
{
}

void ALCGameSession::UnregisterPlayer(const APlayerController* ExitingPlayer)
{
}
