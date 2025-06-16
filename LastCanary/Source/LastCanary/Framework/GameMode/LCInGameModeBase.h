#pragma once

#include "CoreMinimal.h"
#include "Framework/GameMode/LCGameMode.h"
#include "LCInGameModeBase.generated.h"

class AChecklistManager;

UCLASS()
class LASTCANARY_API ALCInGameModeBase : public ALCGameMode
{
	GENERATED_BODY()
	
public:
	ALCInGameModeBase();

	// 언리얼 엔진의 라이프 사이클
	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void InitGameState() override;

	virtual void PostSeamlessTravel() override;
	virtual void HandleSeamlessTravelPlayer(AController*& C) override;
	virtual void StartPlay() override;
	virtual void BeginPlay() override;
	
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;

	// InGameMode Start Game
	void OnAllPlayersJoined();

	virtual void StartGame();

	//virtual void Logout(AController* Exiting) override;


protected:
	UPROPERTY(EditDefaultsOnly, Category = "Checklist")
	TSoftClassPtr<AChecklistManager> ChecklistManagerClass;

public:
	UPROPERTY()
	AChecklistManager* ChecklistManager;

private:
	void InitPlayerAliveCount();
	void CreateCheckListManager();
	void ShowGameLevelInfo();
};
