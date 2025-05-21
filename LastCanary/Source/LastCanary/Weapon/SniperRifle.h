#pragma once

#include "CoreMinimal.h"
#include "Weapon/MainWeapon.h"
#include "SniperRifle.generated.h"

UCLASS()
class LASTCANARY_API ASniperRifle : public AMainWeapon
{
	GENERATED_BODY()
	
public:
	ASniperRifle();

	virtual void EquipParts(AParts* Parts) override;
};
