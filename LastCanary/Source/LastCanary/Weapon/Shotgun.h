#pragma once

#include "CoreMinimal.h"
#include "Weapon/MainWeapon.h"
#include "Shotgun.generated.h"

UCLASS()
class LASTCANARY_API AShotgun : public AMainWeapon
{
	GENERATED_BODY()
	
public:
	AShotgun();

protected:
	void Fire() override;
	virtual void EquipParts(AParts* Parts) override;
};
