#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"
#include "DroneItem.generated.h"

/**
 *
 */
class ABaseDrone;
UCLASS()
class LASTCANARY_API ADroneItem : public AEquipmentItemBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Drone")
	TSubclassOf<ABaseDrone> DroneClass;

	virtual void UseItem() override;
};
