// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AdvancedFriendsGameInstance.h"
#include "OnlineSessionSettings.h"
#include "SessionGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API USessionGameInstance : public UAdvancedFriendsGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UFUNCTION(BlueprintCallable)
	void CreateGameSession();

	void JoinGameSession(const FOnlineSessionSearchResult& SearchResult);
	
	UFUNCTION(BlueprintCallable)
	void JoinGameSessionByIndex(int32 Index);

private:
	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
};
