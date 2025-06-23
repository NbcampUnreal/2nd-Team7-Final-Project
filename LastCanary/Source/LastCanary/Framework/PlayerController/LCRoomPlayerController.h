#pragma once

#include "CoreMinimal.h"
#include "LCPlayerController.h"
#include "DataType/SessionPlayerInfo.h"
#include "DataType/GameResultData.h"
#include "Framework/Manager/ChecklistManager.h"
#include "Character/BasePlayerController.h"
#include "DataType/ItemDropData.h"
#include "LCRoomPlayerController.generated.h"

class URoomWidget;
class AChecklistManager;

UCLASS()
class LASTCANARY_API ALCRoomPlayerController : public ABasePlayerController
{
	GENERATED_BODY()

protected:
	ALCRoomPlayerController();

	virtual void PostSeamlessTravel() override;
	virtual void BeginPlay() override;

public:
	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<class ALCDroneDelivery> DroneDeliveryClass;

	UFUNCTION(Server, Reliable)
	void Server_ShowShopWidget();
	void Server_ShowShopWidget_Implementation();

	UFUNCTION(Client, Reliable)
	void Client_ShowShopWidget(int Gold);
	void Client_ShowShopWidget_Implementation(int Gold);

	UFUNCTION(Server, Reliable)
	void Server_RequestPurchase(const TArray<FItemDropData>& DropList);
	void Server_RequestPurchase_Implementation(const TArray<FItemDropData>& DropList);

};
