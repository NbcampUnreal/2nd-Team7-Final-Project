#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "BaseGameMode.generated.h"


UCLASS()
class LASTCANARY_API ABaseGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ABaseGameMode();

	virtual void BeginPlay() override;
	virtual bool HasCharacterSpawner() const;

	// BattleGMInterface
	virtual void SpawnPlayerCharacter(APlayerController* Controller);
	virtual void HandlePlayerDeath(APlayerController* Controller, const FString& AttackerName);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	// Helper function to get player name from GameInstance
	UFUNCTION(BlueprintCallable, Category = "Player")
	FString GetPlayerNameFromGameInstance(APlayerController* Controller);
	
};