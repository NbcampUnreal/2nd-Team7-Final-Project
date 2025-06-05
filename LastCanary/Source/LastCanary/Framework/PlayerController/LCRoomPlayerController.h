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

UCLASS()
class LASTCANARY_API ALCRoomPlayerController : public ABasePlayerController
{
	GENERATED_BODY()

protected:
	ALCRoomPlayerController();

	virtual void BeginPlay() override;
	virtual void PostSeamlessTravel() override;

public:
	void Client_UpdatePlayerList_Implementation(const TArray<FSessionPlayerInfo>& PlayerInfos) override;
	void UpdatePlayerList(const TArray<FSessionPlayerInfo>& PlayerInfos);

	UPROPERTY(EditDefaultsOnly, Category = "Shop")
	TSubclassOf<class ALCDroneDelivery> DroneDeliveryClass;

	UFUNCTION(Server, Reliable)
	void Server_RequestPurchase(const TArray<FItemDropData>& DropList);
	void Server_RequestPurchase_Implementation(const TArray<FItemDropData>& DropList);

protected:
	UPROPERTY(EditDefaultsOnly, Category = "UI")
	TSubclassOf<URoomWidget> RoomWidgetClass;
	URoomWidget* RoomWidgetInstance;

	void InitInputComponent() override;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Settings|RoomPC", Meta = (DisplayThumbnail = false))
	TObjectPtr<UInputAction> RoomUIAction;

private:
	void CreateRoomWidget();
	UFUNCTION()
	virtual void ToggleShowRoomWidget();

	bool bIsShowRoomUI = false;

	float RePeatRate = 0.1f;

public:
	UFUNCTION(Client, Reliable)
	void Client_NotifyResultReady(const FChecklistResultData& ResultData);
	void Client_NotifyResultReady_Implementation(const FChecklistResultData& ResultData);

	UFUNCTION(Client, Reliable)
	void Client_StartChecklist();
	void Client_StartChecklist_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_MarkPlayerAsEscaped();
	void Server_MarkPlayerAsEscaped_Implementation();

	UFUNCTION(Server, Reliable)
	void Server_RequestSubmitChecklist(const TArray<FChecklistQuestion>& PlayerAnswers);
	void Server_RequestSubmitChecklist_Implementation(const TArray<FChecklistQuestion>& PlayerAnswers);

};
