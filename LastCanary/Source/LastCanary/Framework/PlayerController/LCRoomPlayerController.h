#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "Character/BasePlayerController.h"
#include "LCRoomPlayerController.generated.h"

class URoomWidget;
UCLASS()
class LASTCANARY_API ALCRoomPlayerController : public APlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<UUserWidget> StartGameWidgetClass;

	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URoomWidget> RoomWidgetClass;
	URoomWidget* RoomWidgetInstance;

public:
	UFUNCTION(Client, Reliable)
	void Client_UpdateLobbyUI();

	void Client_UpdateLobbyUI_Implementation();

	// 서버에서만 호출
	void StartGame();

private:
	void CreateAndShowSelecetGameUI();
	void CreateAndShowRoomUI();

};
