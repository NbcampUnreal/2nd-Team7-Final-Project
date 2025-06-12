#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/LCGameMode.h"
#include "GameFramework/GameModeBase.h"
#include "LCRoomGameMode.generated.h"

class AChecklistManager;
UCLASS()
class LASTCANARY_API ALCRoomGameMode : public ALCGameMode
{
	GENERATED_BODY()
	
public:
	int ConnectedPlayers = 0;



	ALCRoomGameMode();

	virtual void BeginPlay() override;
	virtual void PostLogin(APlayerController* NewPlayer) override;
	virtual APlayerController* Login(UPlayer* NewPlayer, ENetRole InRemoteRole, const FString& Portal, const FString& Options, const FUniqueNetIdRepl& UniqueId, FString& ErrorMessage) override;
	virtual void Logout(AController* Exiting) override;
	
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;
	//void OnAllPlayersJoined();
	virtual void PostSeamlessTravel() override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	void StartGame();

	int InGamePlayerNum = 0;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Checklist")
	TSoftClassPtr<AActor> ChecklistManagerClass;

public:
	UPROPERTY()
	AChecklistManager* ChecklistManager;
};
