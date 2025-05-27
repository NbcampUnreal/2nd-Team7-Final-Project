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

	virtual void SpawnPlayerCharacter(APlayerController* Controller);

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	UFUNCTION(BlueprintCallable, Category = "Session")
	void TravelMapBySoftPath(FString SoftPath);
	UFUNCTION(BlueprintCallable, Category = "Session")
	void TravelMapByPath(FString Path);
};