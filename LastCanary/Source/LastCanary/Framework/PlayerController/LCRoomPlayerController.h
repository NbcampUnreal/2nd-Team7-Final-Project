#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "Character/BasePlayerController.h"
#include "LCRoomPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ALCRoomPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartGameWidgetClass;

private:
	void CreateAndShowSelecetGameUI();

public:
	UFUNCTION(Server, Reliable)
	void Server_StartGame();
};
