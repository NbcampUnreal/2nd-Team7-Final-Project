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
};
