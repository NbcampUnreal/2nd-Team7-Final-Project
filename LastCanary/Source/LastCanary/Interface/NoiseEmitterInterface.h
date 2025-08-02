#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "NoiseEmitterInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UNoiseEmitterInterface : public UInterface
{
	GENERATED_BODY()
};


class LASTCANARY_API INoiseEmitterInterface
{
	GENERATED_BODY()

public:
	//UFUNCTION(BlueprintCallable, Category = "Noise")
	virtual float GetCurrentNoiseLevel() const = 0;
};
