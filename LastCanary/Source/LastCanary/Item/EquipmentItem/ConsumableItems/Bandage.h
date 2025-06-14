#pragma once

#include "CoreMinimal.h"
#include "Item/EquipmentItem/EquipmentItemBase.h"

#include "LastCanary.h"

#include "Bandage.generated.h"

/**
 * 
 */
UCLASS()
class LASTCANARY_API ABandage : public AEquipmentItemBase
{
	GENERATED_BODY()
	
public:
	ABandage();

	virtual void UseItem() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Healing")
	float HealingAmount = 20.0f; // 총 회복량

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Healing")
	float HealDuration = 5.0f; // 회복 시간

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Healing")
	int32 MaxDurability = 3; // 최대 내구도
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Healing")
	int32 CurrentDurability;
};
