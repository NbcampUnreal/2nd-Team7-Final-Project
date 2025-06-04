#include "Framework/GameSession/LCGameSession.h"

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

	UE_LOG(LogTemp, Warning, TEXT("Player LogIned !!"));

	NumberOfPlayersInSession++;

}

void ALCGameSession::BeginPlay()
{
    Super::BeginPlay();

    UE_LOG(LogTemp, Display, TEXT("Starting LC Game Session"));
}



void ALCGameSession::NotifyLogout(const APlayerController* ExitingPlayer)
{
    Super::NotifyLogout(ExitingPlayer);

    NumberOfPlayersInSession--;

    //if (NumberOfPlayersInSession == 0)
    //{
    //    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
    //    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();

    //    if (Session->GetSessionState(SessionName) == EOnlineSessionState::InProgress)
    //    {

    //    }
    //}
}

void ALCGameSession::UnregisterPlayer(const APlayerController* ExitingPlayer)
{
    Super::UnregisterPlayer(ExitingPlayer);

}

void ALCGameSession::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);

    //DestroySession();
}


//FName ALCGameSession::GetSessionName()
//{
//    return SessionName;
//}

//void ALCGameSession::DestroySession()
//{
//    UE_LOG(LogTemp, Warning, TEXT("DestroySession!"));
//    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
//    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
//
//    DestroySessionDelegateHandle =
//        Session->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(
//            this,
//            &ThisClass::HandleDestroySessionCompleted));
//
//    if (!Session->DestroySession(SessionName))
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Failed to destroy session."));
//        Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
//        DestroySessionDelegateHandle.Reset();
//    }
//}

//void ALCGameSession::HandleDestroySessionCompleted(FName LCSessionName, bool bWasSuccessful)
//{
//    IOnlineSubsystem* Subsystem = Online::GetSubsystem(GetWorld());
//    IOnlineSessionPtr Session = Subsystem->GetSessionInterface();
//
//    if (bWasSuccessful)
//    {
//        bWasSuccessful = false;
//        //CreateSession();
//        UE_LOG(LogTemp, Log, TEXT("Destroyed session succesfully."));
//    }
//    else
//    {
//        UE_LOG(LogTemp, Warning, TEXT("Failed to destroy session."));
//    }
//
//    Session->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionDelegateHandle);
//    DestroySessionDelegateHandle.Reset();
//}
