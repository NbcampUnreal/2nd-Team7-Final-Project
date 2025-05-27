#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "LCGameSession.generated.h"

UCLASS()
class LASTCANARY_API ALCGameSession : public AGameSession
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ProcessAutoLogin() override;

	virtual FString ApproveLogin(const FString& Options) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void NotifyLogout(const APlayerController* ExitingPlayer) override;

	virtual void RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite = false) override;
	virtual void UnregisterPlayer(const APlayerController* ExitingPlayer) override;

	//FName GetSessionName();

	int MaxNumberOfPlayersInSession = 50;
	int NumberOfPlayersInSession;

	UFUNCTION(BlueprintCallable, Category = "SteamSession")
	void DestroySession();
	void HandleDestroySessionCompleted(FName SessionName, bool bWasSuccessful);
	FDelegateHandle DestroySessionDelegateHandle;
};
