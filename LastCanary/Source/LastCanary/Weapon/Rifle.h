#pragma once

#include "CoreMinimal.h"
#include "Weapon/MainWeapon.h"
#include "Rifle.generated.h"

UCLASS()
class LASTCANARY_API ARifle : public AMainWeapon
{
	GENERATED_BODY()
	
public:
	ARifle();

	virtual void EquipParts(AParts* Parts) override;
};
