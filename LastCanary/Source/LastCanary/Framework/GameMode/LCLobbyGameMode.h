#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LCLobbyGameMode.generated.h"

/**
 * 
 */
class ALCLobbyPlayerController;
UCLASS()
class LASTCANARY_API ALCLobbyGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
public:
	void TryJoinRoom(ALCLobbyPlayerController* RequestingPC, const FString& RoomID, const FString& Password);

	UFUNCTION(BlueprintCallable)
	void StartGame();
};
