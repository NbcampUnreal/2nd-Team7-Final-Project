#include "Multi/Session/SessionGameInstance.h"
#include "OnlineSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"

void USessionGameInstance::Init()
{
	Super::Init();

	IOnlineSubsystem* OnlineSub = IOnlineSubsystem::Get();
	if (!OnlineSub)
	{
		UE_LOG(LogTemp, Error, TEXT("OnlineSubsystem not found!"));
		return;
	}

	SessionInterface = OnlineSub->GetSessionInterface();
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("SessionInterface is invalid!"));
		return;
	}

	SessionInterface->OnCreateSessionCompleteDelegates.AddUObject(this, &USessionGameInstance::OnCreateSessionComplete);
	SessionInterface->OnJoinSessionCompleteDelegates.AddUObject(this, &USessionGameInstance::OnJoinSessionComplete);
}

void USessionGameInstance::CreateGameSession()
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateGameSession failed: Invalid SessionInterface"));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("CreateGameSession failed: Invalid LocalPlayer or NetId"));
		return;
	}

	FOnlineSessionSettings SessionSettings;
	SessionSettings.bIsLANMatch = false;
	SessionSettings.NumPublicConnections = 4;
	SessionSettings.bShouldAdvertise = true;
	SessionSettings.bUsesPresence = true;
	SessionSettings.bUseLobbiesIfAvailable = true;
	SessionSettings.Set(TEXT("VOICECHAT"), true, EOnlineDataAdvertisementType::ViaOnlineService);

	UE_LOG(LogTemp, Log, TEXT("Creating game session..."));
	SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SessionSettings);
}

void USessionGameInstance::JoinGameSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinGameSession failed: Invalid SessionInterface"));
		return;
	}

	const ULocalPlayer* LocalPlayer = GetFirstGamePlayer();
	if (!LocalPlayer || !LocalPlayer->GetPreferredUniqueNetId().IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("JoinGameSession failed: Invalid LocalPlayer or NetId"));
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("Joining game session..."));
	SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult);
}

void USessionGameInstance::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		UE_LOG(LogTemp, Log, TEXT("Session created successfully: %s"), *SessionName.ToString());
		UGameplayStatics::OpenLevel(GetWorld(), FName("L_Als_Playground"), true, TEXT("?listen"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to create session: %s"), *SessionName.ToString());
	}
}

void USessionGameInstance::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!SessionInterface.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("OnJoinSessionComplete failed: Invalid SessionInterface"));
		return;
	}

	FString ConnectString;
	if (SessionInterface->GetResolvedConnectString(SessionName, ConnectString))
	{
		UE_LOG(LogTemp, Log, TEXT("Resolved connect string: %s"), *ConnectString);

		APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
		if (PC)
		{
			PC->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("PlayerController not found"));
		}
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to resolve connect string"));
	}
}

void USessionGameInstance::JoinGameSessionByIndex(int32 Index)
{
	if (!SessionInterface.IsValid() || !SessionSearch.IsValid()) return;

	if (SessionSearch->SearchResults.IsValidIndex(Index))
	{
		JoinGameSession(SessionSearch->SearchResults[Index]);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Invalid session index: %d"), Index);
	}
}