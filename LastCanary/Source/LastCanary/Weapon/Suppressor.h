#pragma once

#include "CoreMinimal.h"
#include "Weapon/Parts.h"
#include "Suppressor.generated.h"

class UNiagaraSystem;

UCLASS()
class LASTCANARY_API ASuppressor : public AParts
{
	GENERATED_BODY()
	
public:
	ASuppressor();

	virtual FVector GetMuzzleSocketLocation();
};
