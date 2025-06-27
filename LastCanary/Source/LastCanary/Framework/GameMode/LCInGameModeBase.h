#pragma once

#include "CoreMinimal.h"
#include "DataTable/BossMonsterRow.h"
#include "Framework/GameMode/LCGameMode.h"
#include "LCInGameModeBase.generated.h"

class AChecklistManager;
class ALCBossSpawner;

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

	virtual void Logout(AController* Exiting) override;

public:
	virtual void InitLCGameMode();

	// InGameMode Start Game
	void OnAllPlayersJoined();
	virtual void StartGame();
	virtual void ClearGame();
	virtual void LoseGame();
	virtual void EndGame();

private:
	void InitBossSpawner();
	ALCBossSpawner* BossSpawner;

protected:
	void ShowGameLevelInfo();

	//void InitMyGameState(int PlayerCount);
	void StartCheckList();
	virtual void CreateBossMonster();
	virtual void CreateCheckListManager();

	UPROPERTY(EditAnywhere, Category = "SafeZone")
	FVector SafeLocation = FVector(0.f, 0.f, 0.f);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Checklist")
	TSubclassOf<AChecklistManager> ChecklistManagerClass;

public:
	UPROPERTY()
	AChecklistManager* ChecklistManager;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	TObjectPtr<UDataTable> BossDataTable;

	FBossMonsterRow* CurrentBossMonsterData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Item Data")
	float ShowLoadingDelay = 1.f;
};
