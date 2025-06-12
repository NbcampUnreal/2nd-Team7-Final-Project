#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "LCGameSession.generated.h"

UCLASS()
class LASTCANARY_API ALCGameSession : public AGameSession
{
	GENERATED_BODY()
	
public:
	virtual FString ApproveLogin(const FString& Options) override;
	virtual void RegisterPlayer(APlayerController* NewPlayer, const FUniqueNetIdRepl& UniqueId, bool bWasFromInvite = false) override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual void BeginPlay() override;

	virtual void NotifyLogout(const APlayerController* ExitingPlayer) override;
	virtual void UnregisterPlayer(const APlayerController* ExitingPlayer) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool KickPlayer(APlayerController* KickedPlayer, const FText& KickReason) override;
};
