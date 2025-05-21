#pragma once

#include "CoreMinimal.h"
#include "Weapon/SubWeapon.h"
#include "HandGun.generated.h"

UCLASS()
class LASTCANARY_API AHandGun : public ASubWeapon
{
	GENERATED_BODY()

public:
	AHandGun();
	
	virtual void EquipParts(AParts* Parts) override;
};
