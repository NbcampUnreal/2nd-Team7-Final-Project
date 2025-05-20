#pragma once

#include "CoreMinimal.h"
#include "Weapon/MeleeWeapon.h"
#include "Fist.generated.h"

class USphereComponent;

UCLASS()
class LASTCANARY_API AFist : public AMeleeWeapon
{
	GENERATED_BODY()
	
public:
	AFist();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Component")
	USphereComponent* FistCoillision;

	void Attack() override;
};
