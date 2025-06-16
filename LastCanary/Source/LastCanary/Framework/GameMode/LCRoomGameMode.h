#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/LCGameMode.h"
#include "LCRoomGameMode.generated.h"

class AChecklistManager;
UCLASS()
class LASTCANARY_API ALCRoomGameMode : public ALCGameMode
{
	GENERATED_BODY()
	
public:
	ALCRoomGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;

	virtual void PostSeamlessTravel() override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void StartPlay() override;
	virtual void BeginPlay() override;

	void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

public:
	void UpdatePlayers() override;

private:
	//void StartGame();
	void HandleBaseCampDoors();

};
