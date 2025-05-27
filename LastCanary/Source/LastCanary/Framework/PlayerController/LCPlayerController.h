#pragma once

#include "CoreMinimal.h"
#include "UI/Manager/LCUIManager.h"
#include "GameFramework/PlayerController.h"
#include "LCPlayerController.generated.h"

class ULCUIManager;

UCLASS()
class LASTCANARY_API ALCPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

	UFUNCTION()
	void Login();

	UFUNCTION(Client, Reliable)
	void Client_UpdatePlayers();

	virtual void Client_UpdatePlayers_Implementation();

	void StartGame(FString SoftPath);

protected:
	TObjectPtr<ULCUIManager> LCUIManager;
};
