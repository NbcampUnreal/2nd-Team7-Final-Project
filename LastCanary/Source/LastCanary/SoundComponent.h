#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "SoundComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LASTCANARY_API USoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	USoundComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

		
};
