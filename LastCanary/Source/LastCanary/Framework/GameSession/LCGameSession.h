// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "LCGameSession.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCGameSession : public AGameSession
{
	GENERATED_BODY()
	
public:
	ALCGameSession();

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual bool ProcessAutoLogin() override;

	virtual FString ApproveLogin(const FString& Options) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void NotifyLogout(const APlayerController* ExitingPlayer) override;

	virtual void RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite = false) override;
	virtual void UnregisterPlayer(const APlayerController* ExitingPlayer) override;

	FName SessionName = "SessionName";
	int MaxNumberOfPlayersInSession = 4;
	int NumberOfPlayersInSession;
};
