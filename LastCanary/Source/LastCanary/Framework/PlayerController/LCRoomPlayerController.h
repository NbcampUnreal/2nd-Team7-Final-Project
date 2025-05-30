#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "DataType/SessionPlayerInfo.h"
#include "Character/BasePlayerController.h"
#include "DataType/ItemDropData.h"
#include "LCRoomPlayerController.generated.h"

class URoomWidget;

UCLASS()
class LASTCANARY_API ALCRoomPlayerController : public ABasePlayerController
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URoomWidget> RoomWidgetClass;
	URoomWidget* RoomWidgetInstance;

public:
	void Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos) override;
	void Client_UpdatePlayers_Implementation() override;

	// 서버에서만 호출
	//void StartGame();

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<class ALCDroneDelivery> DroneDeliveryClass;

	UFUNCTION(Server, Reliable)
	void Server_RequestPurchase(const TArray<FItemDropData>& DropList);
	void Server_RequestPurchase_Implementation(const TArray<FItemDropData>& DropList);

private:
	void CreateAndShowRoomUI();

};
