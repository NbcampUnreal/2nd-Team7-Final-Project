#pragma once

#include "CoreMinimal.h"
#include "Weapon/MainWeapon.h"
#include "SubMachineGun.generated.h"

UCLASS()
class LASTCANARY_API ASubMachineGun : public AMainWeapon
{
	GENERATED_BODY()
	
public:
	ASubMachineGun();

	virtual void EquipParts(AParts* Parts) override;
};
