#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "WalkieTalkie.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API AWalkieTalkie : public AEquipmentItemBase
{
	GENERATED_BODY()

public:
	virtual void UseItem() override;
	
	UFUNCTION(BlueprintImplementableEvent)
	void StartWalkieTalkie();

	UFUNCTION(Server, Reliable, Category = "WalkieTalkie")
	void Server_UseWalkieTalkie(APlayerController* UserController);
	void Server_UseWalkieTalkie_Implementation(APlayerController* UserController);

	UFUNCTION(Client, Reliable, Category = "WalkieTalkie")
	void Client_StartWalkieTalkie();
	void Client_StartWalkieTalkie_Implementation();
};
