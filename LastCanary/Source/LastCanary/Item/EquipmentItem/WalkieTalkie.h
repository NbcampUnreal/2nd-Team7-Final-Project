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

	UFUNCTION(BlueprintCallable)
	void StopWalkieTalkie();

	UFUNCTION(Server, Reliable, Category = "WalkieTalkie")
	void Server_UseWalkieTalkie();
	void Server_UseWalkieTalkie_Implementation();

	UFUNCTION(Server, Reliable, Category = "WalkieTalkie")
	void Server_StopWalkieTalkie();
	void Server_StopWalkieTalkie_Implementation();

	UFUNCTION(Client, Reliable, Category = "WalkieTalkie")
	void Client_StartWalkieTalkie();
	void Client_StartWalkieTalkie_Implementation();

	UFUNCTION(Client, Reliable, Category = "WalkieTalkie")
	void Client_StopWalkieTalkie();
	void Client_StopWalkieTalkie_Implementation();

	UPROPERTY(BlueprintReadWrite, Replicated)
	bool bUseWalkie;

	UFUNCTION(BlueprintPure)
	bool IsWalkieTalkieActive() const { return bUseWalkie; }

protected:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
